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
// $Id: RomWidget.cxx 2318 2011-12-31 21:56:36Z stephena $
//
//   Based on code from ScummVM - Scumm Interpreter
//   Copyright (C) 2002-2004 The ScummVM project
//============================================================================

#include <sstream>

#include "Debugger.hxx"
#include "DebuggerParser.hxx"
#include "CartDebug.hxx"
#include "DiStella.hxx"
#include "CpuDebug.hxx"
#include "GuiObject.hxx"
#include "InputTextDialog.hxx"
#include "DataGridWidget.hxx"
#include "EditTextWidget.hxx"
#include "PopUpWidget.hxx"
#include "StringList.hxx"
#include "ContextMenu.hxx"
#include "RomListWidget.hxx"
#include "RomWidget.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
RomWidget::RomWidget(GuiObject* boss, const GUI::Font& font, int x, int y)
  : Widget(boss, font, x, y, 16, 16),
    CommandSender(boss),
    myListIsDirty(true),
    myCurrentBank(-1)
{
  _type = kRomWidget;

  int xpos, ypos;
  StaticTextWidget* t;
  WidgetArray wid;

  // Show current bank
  xpos = x;  ypos = y + 7;
  ostringstream buf;
  buf << "Current bank (" << dec
      << instance().debugger().cartDebug().bankCount() << " total):";
  t = new StaticTextWidget(boss, font, xpos, ypos,
                           font.getStringWidth(buf.str()),
                           font.getFontHeight(),
                           buf.str(), kTextAlignLeft);

  xpos += t->getWidth() + 5;
  myBank = new DataGridWidget(boss, font, xpos, ypos-2,
                              1, 1, 4, 8, kBASE_10);
  myBank->setTarget(this);
  myBank->setRange(0, instance().debugger().cartDebug().bankCount());
  if(instance().debugger().cartDebug().bankCount() <= 1)
    myBank->setEditable(false);
  addFocusWidget(myBank);

  // 'resolvedata' setting for Distella
  xpos += myBank->getWidth() + 20;
  StringMap items;
  items.push_back("Never", "never");
  items.push_back("Always", "always");
  items.push_back("Automatic", "auto");
  myResolveData =
    new PopUpWidget(boss, font, xpos, ypos-2, font.getStringWidth("Automatic"),
                    font.getLineHeight(), items,
                    "Resolve data: ", font.getStringWidth("Resolve data: "),
                    kResolveDataChanged);
  myResolveData->setTarget(this);
  addFocusWidget(myResolveData);

  // Create rom listing
  xpos = x;  ypos += myBank->getHeight() + 4;
  const GUI::Rect& dialog = instance().debugger().getDialogBounds();
  int w = dialog.width() - x - 5, h = dialog.height() - ypos - 3;

  myRomList = new RomListWidget(boss, font, xpos, ypos, w, h);
  myRomList->setTarget(this);
  myRomList->myMenu->setTarget(this);
  addFocusWidget(myRomList);

  // Calculate real dimensions
  _w = myRomList->getWidth();
  _h = myRomList->getHeight();

  // Create dialog box for save ROM (get name)
  StringList label;
  label.push_back("Filename: ");
  mySaveRom = new InputTextDialog(boss, font, label);
  mySaveRom->setTarget(this);

  // By default, we try to automatically determine code vs. data sections
  myResolveData->setSelected(instance().settings().getString("resolvedata"), "auto");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
RomWidget::~RomWidget()
{
  delete mySaveRom;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void RomWidget::loadConfig()
{
  Debugger& dbg = instance().debugger();
  CartDebug& cart = dbg.cartDebug();
  bool bankChanged = myCurrentBank != cart.getBank();
  myCurrentBank = cart.getBank();

  // Fill romlist the current bank of source or disassembly
  myListIsDirty |= cart.disassemble(myResolveData->getSelectedTag(), myListIsDirty);
  if(myListIsDirty)
  {
    myRomList->setList(cart.disassembly(), dbg.breakpoints());
    myListIsDirty = false;
  }

  // Update romlist to point to current PC (if it has changed)
  int pcline = cart.addressToLine(dbg.cpuDebug().pc());
  if(pcline >= 0 && pcline != myRomList->getHighlighted())
    myRomList->setHighlighted(pcline);

  // Set current bank and number of banks
  myBank->setList(-1, myCurrentBank, bankChanged);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void RomWidget::handleCommand(CommandSender* sender, int cmd, int data, int id)
{
  switch(cmd)
  {
    case kRLBreakpointChangedCmd:
      // 'id' is the line in the disassemblylist to be accessed
      // 'data' is the state of the breakpoint at 'id'
      setBreak(id, data);
      // Refresh the romlist, since the breakpoint may not have
      // actually changed
      myRomList->setDirty();
      myRomList->draw();
      break;

    case kRLRomChangedCmd:
      // 'data' is the line in the disassemblylist to be accessed
      patchROM(data, myRomList->getEditString());
      break;

    case kCMenuItemSelectedCmd:
    {
      const string& rmb = myRomList->myMenu->getSelectedTag();

      if(rmb == "saverom")
      {
        mySaveRom->show(_x + 50, _y + 80);
        mySaveRom->setEditString("");
        mySaveRom->setTitle("");
        mySaveRom->setEmitSignal(kRomNameEntered);
      }
      else if(rmb == "setpc")
        setPC(myRomList->getSelected());
      else if(rmb == "runtopc")
        runtoPC(myRomList->getSelected());
      else if(rmb == "disasm")
        invalidate();
      else if(rmb == "pcaddr")
      {
        DiStella::settings.show_addresses = !DiStella::settings.show_addresses;
        instance().settings().setString("showaddr",
            DiStella::settings.show_addresses ? "true" : "false");
        invalidate();
      }
      else if(rmb == "gfx")
      {
        if(DiStella::settings.gfx_format == kBASE_16)
        {
          DiStella::settings.gfx_format = kBASE_2;
          instance().settings().setString("gfxformat", "2");
        }
        else
        {
          DiStella::settings.gfx_format = kBASE_16;
          instance().settings().setString("gfxformat", "16");
        }
        invalidate();
      }
      break;  // kCMenuItemSelectedCmd
    }

    case kDGItemDataChangedCmd:
      setBank(myBank->getSelectedValue());
      break;

    case kResolveDataChanged:
      instance().settings().setString("resolvedata", myResolveData->getSelectedTag());
      invalidate();
      loadConfig();
      break;

    case kRomNameEntered:
    {
      const string& rom = mySaveRom->getResult();
      if(rom == "")
        mySaveRom->setTitle("Invalid name");
      else
      {
        saveROM(rom);
        parent().removeDialog();
      }
      break;
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void RomWidget::setBank(uInt16 bank)
{
  ostringstream command;
  command << "bank #" << bank;
  instance().debugger().run(command.str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void RomWidget::setBreak(int disasm_line, bool state)
{
  const CartDebug::DisassemblyList& list =
      instance().debugger().cartDebug().disassembly().list;
  if(disasm_line >= (int)list.size())  return;

  if(list[disasm_line].address != 0 && list[disasm_line].bytes != "")
    instance().debugger().setBreakPoint(list[disasm_line].address, state);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void RomWidget::setPC(int disasm_line)
{
  const CartDebug::DisassemblyList& list =
      instance().debugger().cartDebug().disassembly().list;
  if(disasm_line >= (int)list.size())  return;

  if(list[disasm_line].address != 0)
  {
    ostringstream command;
    command << "pc #" << list[disasm_line].address;
    instance().debugger().run(command.str());
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void RomWidget::runtoPC(int disasm_line)
{
  const CartDebug::DisassemblyList& list =
      instance().debugger().cartDebug().disassembly().list;
  if(disasm_line >= (int)list.size())  return;

  if(list[disasm_line].address != 0)
  {
    ostringstream command;
    command << "runtopc #" << list[disasm_line].address;
    instance().debugger().run(command.str());
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void RomWidget::patchROM(int disasm_line, const string& bytes)
{
  const CartDebug::DisassemblyList& list =
      instance().debugger().cartDebug().disassembly().list;
  if(disasm_line >= (int)list.size())  return;

  if(list[disasm_line].address != 0)
  {
    ostringstream command;

    // Temporarily set to correct base, so we don't have to prefix each byte
    // with the type of data
    BaseFormat oldbase = instance().debugger().parser().base();
    if(list[disasm_line].type == CartDebug::GFX)
      instance().debugger().parser().setBase(DiStella::settings.gfx_format);
    else
      instance().debugger().parser().setBase(kBASE_16);

    command << "rom #" << list[disasm_line].address << " " << bytes;
    instance().debugger().run(command.str());

    // Restore previous base
    instance().debugger().parser().setBase(oldbase);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void RomWidget::saveROM(const string& rom)
{
  ostringstream command;
  command << "saverom " << rom;
  instance().debugger().run(command.str());
}
