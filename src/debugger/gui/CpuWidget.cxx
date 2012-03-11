//============================================================================
//
//   SSSS    tt          lll  lll       
//  SS  SS   tt           ll   ll        
//  SS     tttttt  eeee   ll   ll   aaaa 
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2012 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: CpuWidget.cxx 2318 2011-12-31 21:56:36Z stephena $
//
//   Based on code from ScummVM - Scumm Interpreter
//   Copyright (C) 2002-2004 The ScummVM project
//============================================================================

#include <sstream>

#include "OSystem.hxx"
#include "GuiObject.hxx"
#include "Debugger.hxx"
#include "CartDebug.hxx"
#include "CpuDebug.hxx"
#include "Widget.hxx"
#include "DataGridWidget.hxx"
#include "EditTextWidget.hxx"
#include "ToggleBitWidget.hxx"

#include "CpuWidget.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CpuWidget::CpuWidget(GuiObject* boss, const GUI::Font& font, int x, int y)
  : Widget(boss, font, x, y, 16, 16),
    CommandSender(boss)
{
  _type = kCpuWidget;

  const int fontWidth  = font.getMaxCharWidth(),
            fontHeight = font.getFontHeight(),
            lineHeight = font.getLineHeight();
  int xpos, ypos, lwidth;

  // Create a 1x1 grid with label for the PC register
  xpos = x;  ypos = y;  lwidth = 4 * fontWidth;
  new StaticTextWidget(boss, font, xpos, ypos+2, lwidth-2, fontHeight,
                       "PC:", kTextAlignLeft);
  myPCGrid =
    new DataGridWidget(boss, font, xpos + lwidth, ypos, 1, 1, 4, 16, kBASE_16);
  myPCGrid->setTarget(this);
  myPCGrid->setID(kPCRegID);
  addFocusWidget(myPCGrid);

  // Create a read-only textbox containing the current PC label
  xpos += lwidth + myPCGrid->getWidth() + 10;
  myPCLabel = new EditTextWidget(boss, font, xpos, ypos, fontWidth*25,
                                 lineHeight, "");
  myPCLabel->setEditable(false);

  // Create a 1x4 grid with labels for the other CPU registers
  xpos = x + lwidth;  ypos += myPCGrid->getHeight() + 1;
  myCpuGrid =
    new DataGridWidget(boss, font, xpos, ypos, 1, 4, 2, 8, kBASE_16);
  myCpuGrid->setTarget(this);
  myCpuGrid->setID(kCpuRegID);
  addFocusWidget(myCpuGrid);

  // Create a 1x4 grid with decimal and binary values for the other CPU registers
  xpos = x + lwidth + myPCGrid->getWidth() + 10;
  myCpuGridDecValue = 
    new DataGridWidget(boss, font, xpos, ypos, 1, 4, 3, 8, kBASE_10);
  myCpuGridDecValue->setEditable(false);
  xpos += myCpuGridDecValue->getWidth() + 5;
  myCpuGridBinValue = 
    new DataGridWidget(boss, font, xpos, ypos, 1, 4, 8, 8, kBASE_2);
  myCpuGridBinValue->setEditable(false);

  // Create a label and 1x3 grid showing the source of data for A/X/Y registers
  xpos += myCpuGridBinValue->getWidth() + 20;
  myCpuDataSrcGrid = 
    new DataGridWidget(boss, font, xpos, ypos, 1, 4, 4, 16, kBASE_16);
  myCpuDataSrcGrid->setEditable(false);
  new StaticTextWidget(boss, font, xpos-font.getMaxCharWidth(),
                       ypos+myCpuDataSrcGrid->getHeight() + 4,
                       font.getStringWidth("Src Addr"), fontHeight, "Src Addr",
                       kTextAlignLeft);

  // Add labels for other CPU registers
  xpos = x;
  string labels[4] = { "SP:", "A:", "X:", "Y:" };
  for(int row = 0; row < 4; ++row)
  {
    new StaticTextWidget(boss, font, xpos, ypos + row*lineHeight + 2,
                         lwidth-2, fontHeight,
                         labels[row], kTextAlignLeft);
  }

  // Create a bitfield widget for changing the processor status
  xpos = x;  ypos += 4*lineHeight + 2;
  new StaticTextWidget(boss, font, xpos, ypos+2, lwidth-2, fontHeight,
                       "PS:", kTextAlignLeft);
  myPSRegister = new ToggleBitWidget(boss, font, xpos+lwidth, ypos, 8, 1);
  myPSRegister->setTarget(this);
  addFocusWidget(myPSRegister);

  // Set the strings to be used in the PSRegister
  // We only do this once because it's the state that changes, not the strings
  const char* offstr[] = { "n", "v", "-", "b", "d", "i", "z", "c" };
  const char* onstr[]  = { "N", "V", "-", "B", "D", "I", "Z", "C" };
  StringList off, on;
  for(int i = 0; i < 8; ++i)
  {
    off.push_back(offstr[i]);
    on.push_back(onstr[i]);
  }
  myPSRegister->setList(off, on);

  // Calculate real dimensions
  _w = lwidth + myPCGrid->getWidth() + myPCLabel->getWidth() + 20;
  _h = ypos + myPSRegister->getHeight() - y;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CpuWidget::~CpuWidget()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CpuWidget::handleCommand(CommandSender* sender, int cmd, int data, int id)
{
  int addr = -1, value = -1;
  CpuDebug& dbg = instance().debugger().cpuDebug();

  switch(cmd)
  {
    case kDGItemDataChangedCmd:
      switch(id)
      {
        case kPCRegID:
          addr  = myPCGrid->getSelectedAddr();
          value = myPCGrid->getSelectedValue();
          break;

        case kCpuRegID:
          addr  = myCpuGrid->getSelectedAddr();
          value = myCpuGrid->getSelectedValue();
          break;
      }

      switch(addr)
      {
        case kPCRegAddr:
        {
          // Use the parser to set PC, since we want to propagate the
          // event the rest of the debugger widgets
          ostringstream command;
          command << "pc #" << value;
          instance().debugger().run(command.str());
          break;
        }

        case kSPRegAddr:
          dbg.setSP(value);
          myCpuGridDecValue->setValue(0, value);
          myCpuGridBinValue->setValue(0, value);
          break;

        case kARegAddr:
          dbg.setA(value);
          myCpuGridDecValue->setValue(1, value);
          myCpuGridBinValue->setValue(1, value);
          break;

        case kXRegAddr:
          dbg.setX(value);
          myCpuGridDecValue->setValue(2, value);
          myCpuGridBinValue->setValue(2, value);
          break;

        case kYRegAddr:
          dbg.setY(value);
          myCpuGridDecValue->setValue(3, value);
          myCpuGridBinValue->setValue(3, value);
          break;
      }
      break;

    case kTWItemDataChangedCmd:
    {
      bool state = myPSRegister->getSelectedState();

      switch(data)
      {
        case kPSRegN:
          dbg.setN(state);
          break;

        case kPSRegV:
          dbg.setV(state);
          break;

        case kPSRegB:
          dbg.setB(state);
          break;

        case kPSRegD:
          dbg.setD(state);
          break;

        case kPSRegI:
          dbg.setI(state);
          break;

        case kPSRegZ:
          dbg.setZ(state);
          break;

        case kPSRegC:
          dbg.setC(state);
          break;
      }
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CpuWidget::loadConfig()
{
  fillGrid();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CpuWidget::setOpsWidget(DataGridOpsWidget* w)
{
  myPCGrid->setOpsWidget(w);
  myCpuGrid->setOpsWidget(w);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CpuWidget::fillGrid()
{
  IntArray alist;
  IntArray vlist;
  BoolArray changed;

  // We push the enumerated items as addresses, and deal with the real
  // address in the callback (handleCommand)
  Debugger& dbg = instance().debugger();
  CpuDebug& cpu = dbg.cpuDebug();
  const CpuState& state    = (CpuState&) cpu.getState();
  const CpuState& oldstate = (CpuState&) cpu.getOldState();

  // Add PC to its own DataGridWidget
  alist.push_back(kPCRegAddr);
  vlist.push_back(state.PC);
  changed.push_back(state.PC != oldstate.PC);

  myPCGrid->setList(alist, vlist, changed);

  // Add the other registers
  alist.clear(); vlist.clear(); changed.clear();
  alist.push_back(kSPRegAddr);
  alist.push_back(kARegAddr);
  alist.push_back(kXRegAddr);
  alist.push_back(kYRegAddr);

  // And now fill the values
  vlist.push_back(state.SP);
  vlist.push_back(state.A);
  vlist.push_back(state.X);
  vlist.push_back(state.Y);

  // Figure out which items have changed
  changed.push_back(state.SP != oldstate.SP);
  changed.push_back(state.A  != oldstate.A);
  changed.push_back(state.X  != oldstate.X);
  changed.push_back(state.Y  != oldstate.Y);

  // Finally, update the register list
  myCpuGrid->setList(alist, vlist, changed);
  myCpuGridDecValue->setList(alist, vlist, changed);
  myCpuGridBinValue->setList(alist, vlist, changed);

  // Update the data sources for the A/X/Y registers
  alist.clear(); vlist.clear(); changed.clear();
  alist.push_back(0);
  alist.push_back(0);
  alist.push_back(0);

  vlist.push_back(state.srcS);
  vlist.push_back(state.srcA);
  vlist.push_back(state.srcX);
  vlist.push_back(state.srcY);

  changed.push_back(state.srcS  != oldstate.srcS);
  changed.push_back(state.srcA  != oldstate.srcA);
  changed.push_back(state.srcX  != oldstate.srcX);
  changed.push_back(state.srcY  != oldstate.srcY);
  myCpuDataSrcGrid->setList(alist, vlist, changed);

  // Update the PS register booleans
  changed.clear();
  for(unsigned int i = 0; i < state.PSbits.size(); ++i)
    changed.push_back(state.PSbits[i] != oldstate.PSbits[i]);

  myPSRegister->setState(state.PSbits, changed);
  myPCLabel->setEditString(dbg.cartDebug().getLabel(state.PC, true));
}
