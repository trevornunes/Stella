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
// $Id: DebuggerDialog.cxx 2356 2012-01-14 22:00:54Z stephena $
//
//   Based on code from ScummVM - Scumm Interpreter
//   Copyright (C) 2002-2004 The ScummVM project
//============================================================================

#include "Widget.hxx"
#include "Dialog.hxx"
#include "TabWidget.hxx"
#include "TiaInfoWidget.hxx"
#include "TiaOutputWidget.hxx"
#include "TiaZoomWidget.hxx"
#include "AudioWidget.hxx"
#include "PromptWidget.hxx"
#include "CpuWidget.hxx"
#include "RamWidget.hxx"
#include "RiotWidget.hxx"
#include "RomWidget.hxx"
#include "TiaWidget.hxx"
#include "DataGridOpsWidget.hxx"
#include "EditTextWidget.hxx"
#include "MessageBox.hxx"
#include "Rect.hxx"
#include "Debugger.hxx"
#include "DebuggerParser.hxx"

#include "DebuggerDialog.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DebuggerDialog::DebuggerDialog(OSystem* osystem, DialogContainer* parent,
                               int x, int y, int w, int h)
  : Dialog(osystem, parent, x, y, w, h, true),  // use base surface
    myTab(NULL),
    myFatalError(NULL)
{
  addTiaArea();
  addTabArea();
  addStatusArea();
  addRomArea();

  // Inform the output widget about its associated zoom widget
  myTiaOutput->setZoomWidget(myTiaZoom);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DebuggerDialog::~DebuggerDialog()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::loadConfig()
{
  myTab->loadConfig();
  myTiaInfo->loadConfig();
  myTiaOutput->loadConfig();
  myTiaZoom->loadConfig();
  myCpu->loadConfig();
  myRam->loadConfig();
  myRom->loadConfig();

  myMessageBox->setEditString("");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::handleKeyDown(StellaKey key, StellaMod mod, char ascii)
{
  bool handled = instance().eventHandler().kbdAlt(mod);
  if(handled)
  {
    switch(ascii)
    {
      case 's':
        doStep();
        break;
      case 't':
        doTrace();
        break;
      case 'f':
        doAdvance();
        break;
      case 'l':
        doScanlineAdvance();
        break;
      case 'r':
        doRewind();
        break;
      default:
        handled = false;
        break;
    }
  }
  if(!handled)
    Dialog::handleKeyDown(key, mod, ascii);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::handleCommand(CommandSender* sender, int cmd,
                                   int data, int id)
{
  // We reload the tabs in the cases where the actions could possibly
  // change their contents
  switch(cmd)
  {
    case kDDStepCmd:
      doStep();
      break;

    case kDDTraceCmd:
      doTrace();
      break;

    case kDDAdvCmd:
      doAdvance();
      break;

    case kDDSAdvCmd:
      doScanlineAdvance();
      break;

    case kDDRewindCmd:
      doRewind();
      break;

    case kDDExitCmd:
      doExitDebugger();
      break;

    case kDDExitFatalCmd:
      doExitRom();
      break;

    default:
      Dialog::handleCommand(sender, cmd, data, id);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::showFatalMessage(const string& msg)
{
  const GUI::Rect& r = instance().debugger().getDialogBounds();

  delete myFatalError;
  myFatalError =
    new MessageBox(this, instance().consoleFont(), msg,
                   r.width(), r.height(), kDDExitFatalCmd,
                   "Exit ROM", "Continue");
  myFatalError->show();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::addTiaArea()
{
  const GUI::Rect& r = instance().debugger().getTiaBounds();

  myTiaOutput = new TiaOutputWidget(this, instance().consoleFont(),
                                    r.left, r.top, r.width(), r.height());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::addTabArea()
{
  const GUI::Rect& r = instance().debugger().getTabBounds();

  const int vBorder = 4;

  // The tab widget
  myTab = new TabWidget(this, instance().consoleFont(), r.left, r.top + vBorder,
                        r.width(), r.height() - vBorder);
  addTabWidget(myTab);

  const int widWidth  = r.width() - vBorder;
  const int widHeight = r.height() - myTab->getTabHeight() - vBorder - 4;
  int tabID;

  // The Prompt/console tab
  tabID = myTab->addTab("Prompt");
  myPrompt = new PromptWidget(myTab, instance().consoleFont(),
                              2, 2, widWidth, widHeight);
  myTab->setParentWidget(tabID, myPrompt);
  addToFocusList(myPrompt->getFocusList(), tabID);

  // The TIA tab
  tabID = myTab->addTab("TIA");
  TiaWidget* tia = new TiaWidget(myTab, instance().consoleFont(),
                                 2, 2, widWidth, widHeight);
  myTab->setParentWidget(tabID, tia);
  addToFocusList(tia->getFocusList(), tabID);

  // The input/output tab (includes RIOT and INPTx from TIA)
  tabID = myTab->addTab("I/O");
  RiotWidget* riot = new RiotWidget(myTab, instance().consoleFont(),
                                    2, 2, widWidth, widHeight);
  myTab->setParentWidget(tabID, riot);
  addToFocusList(riot->getFocusList(), tabID);

  // The Audio tab
  tabID = myTab->addTab("Audio");
  AudioWidget* aud = new AudioWidget(myTab, instance().consoleFont(),
                                     2, 2, widWidth, widHeight);
  myTab->setParentWidget(tabID, aud);
  addToFocusList(aud->getFocusList(), tabID);

  myTab->setActiveTab(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::addStatusArea()
{
  const GUI::Font& font = instance().consoleFont();
  const int lineHeight = font.getLineHeight();
  const GUI::Rect& r = instance().debugger().getStatusBounds();
  int xpos, ypos;

  xpos = r.left;  ypos = r.top;
  myTiaInfo = new TiaInfoWidget(this, instance().consoleFont(), xpos, ypos);

  ypos += myTiaInfo->getHeight() + 10;
  myTiaZoom = new TiaZoomWidget(this, instance().consoleFont(), xpos+10, ypos,
                                r.width()-10, r.height()-lineHeight-ypos-10);
  addToFocusList(myTiaZoom->getFocusList());

  xpos += 10;  ypos += myTiaZoom->getHeight() + 10;
  myMessageBox = new EditTextWidget(this, instance().consoleFont(),
                                    xpos, ypos, myTiaZoom->getWidth(),
                                    font.getLineHeight(), "");
  myMessageBox->setEditable(false);
  myMessageBox->clearFlags(WIDGET_RETAIN_FOCUS);
  myMessageBox->setTextColor(kTextColorEm);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::addRomArea()
{
  const GUI::Rect& r = instance().debugger().getRomBounds();
  int xpos, ypos;

  xpos = r.left + 10;  ypos = 10;
  myCpu = new CpuWidget(this, instance().consoleFont(), xpos, ypos);
  addToFocusList(myCpu->getFocusList());

  xpos = r.left + 10;  ypos += myCpu->getHeight() + 10;
  myRam = new RamWidget(this, instance().consoleFont(), xpos, ypos);
  addToFocusList(myRam->getFocusList());

  xpos = r.left + 10 + myCpu->getWidth() + 5;
  DataGridOpsWidget* ops = new DataGridOpsWidget(this, instance().consoleFont(),
                                                 xpos, 20);

  int bwidth  = instance().consoleFont().getStringWidth("Frame +1 "),
      bheight = instance().consoleFont().getLineHeight() + 2;
  int buttonX = r.right - bwidth - 5, buttonY = r.top + 5;
  new ButtonWidget(this, instance().consoleFont(), buttonX, buttonY,
                   bwidth, bheight, "Step", kDDStepCmd);
  buttonY += bheight + 4;
  new ButtonWidget(this, instance().consoleFont(), buttonX, buttonY,
                   bwidth, bheight, "Trace", kDDTraceCmd);
  buttonY += bheight + 4;
  new ButtonWidget(this, instance().consoleFont(), buttonX, buttonY,
                   bwidth, bheight, "Scan +1", kDDSAdvCmd);
  buttonY += bheight + 4;
  new ButtonWidget(this, instance().consoleFont(), buttonX, buttonY,
                   bwidth, bheight, "Frame +1", kDDAdvCmd);
  buttonY += bheight + 4;
  new ButtonWidget(this, instance().consoleFont(), buttonX, buttonY,
                   bwidth, bheight, "Exit", kDDExitCmd);

  bwidth = instance().consoleFont().getStringWidth("< ") + 4;
  bheight = bheight * 5 + 4*4;
  buttonX -= (bwidth + 5);
  buttonY = r.top + 5;
  myRewindButton =
    new ButtonWidget(this, instance().consoleFont(), buttonX, buttonY,
                     bwidth, bheight, "<", kDDRewindCmd);
  myRewindButton->clearFlags(WIDGET_ENABLED);

  xpos = r.left + 10;  ypos += myRam->getHeight() + 5;
  myRom = new RomWidget(this, instance().consoleFont(), xpos, ypos);
  addToFocusList(myRom->getFocusList());

  // Add the DataGridOpsWidget to any widgets which contain a
  // DataGridWidget which we want controlled
  myCpu->setOpsWidget(ops);
  myRam->setOpsWidget(ops);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::doStep()
{
  instance().debugger().parser().run("step");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::doTrace()
{
  instance().debugger().parser().run("trace");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::doAdvance()
{
  instance().debugger().parser().run("frame #1");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::doScanlineAdvance()
{
  instance().debugger().parser().run("scanline #1");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::doRewind()
{
  instance().debugger().parser().run("rewind");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::doExitDebugger()
{
  instance().debugger().parser().run("run");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DebuggerDialog::doExitRom()
{
  instance().debugger().parser().run("exitrom");
}
