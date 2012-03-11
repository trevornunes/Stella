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
// $Id: DataGridOpsWidget.hxx 2318 2011-12-31 21:56:36Z stephena $
//
//   Based on code from ScummVM - Scumm Interpreter
//   Copyright (C) 2002-2004 The ScummVM project
//============================================================================

#ifndef DATA_GRID_OPS_WIDGET_HXX
#define DATA_GRID_OPS_WIDGET_HXX

#include "Widget.hxx"
#include "Command.hxx"

// DataGridWidget operations
enum {
  kDGZeroCmd   = 'DGze',
  kDGInvertCmd = 'DGiv',
  kDGNegateCmd = 'DGng',
  kDGIncCmd    = 'DGic',
  kDGDecCmd    = 'DGdc',
  kDGShiftLCmd = 'DGls',
  kDGShiftRCmd = 'DGrs'
};

class DataGridOpsWidget : public Widget, public CommandSender
{
  public:
    DataGridOpsWidget(GuiObject* boss, const GUI::Font& font, int x, int y);
    virtual ~DataGridOpsWidget() {}

    void setTarget(CommandReceiver* target);
    void setEnabled(bool e);

  private:
    ButtonWidget* _zeroButton;
    ButtonWidget* _invButton;
    ButtonWidget* _negButton;
    ButtonWidget* _incButton;
    ButtonWidget* _decButton;
    ButtonWidget* _shiftLeftButton;
    ButtonWidget* _shiftRightButton;
};

#endif
