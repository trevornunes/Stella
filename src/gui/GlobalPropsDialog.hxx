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
// $Id: GlobalPropsDialog.hxx 2318 2011-12-31 21:56:36Z stephena $
//
//   Based on code from ScummVM - Scumm Interpreter
//   Copyright (C) 2002-2004 The ScummVM project
//============================================================================

#ifndef GLOBAL_PROPS_DIALOG_HXX
#define GLOBAL_PROPS_DIALOG_HXX

class CommandSender;
class DialogContainer;
class CheckboxWidget;
class PopUpWidget;
class OSystem;

#include "Dialog.hxx"
#include "bspf.hxx"

class GlobalPropsDialog : public Dialog
{
  public:
    GlobalPropsDialog(GuiObject* boss, const GUI::Font& font);
    virtual ~GlobalPropsDialog();

  private:
    void loadConfig();
    void saveConfig();
    void setDefaults();

    virtual void handleCommand(CommandSender* sender, int cmd, int data, int id);

  private:
    PopUpWidget*    myBSType;
    PopUpWidget*    myLeftDiff;
    PopUpWidget*    myRightDiff;
    PopUpWidget*    myTVType;

    CheckboxWidget* myHoldSelect;
    CheckboxWidget* myHoldReset;
    CheckboxWidget* myHoldButton0;
};

#endif
