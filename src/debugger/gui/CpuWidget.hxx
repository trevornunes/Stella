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
// $Id: CpuWidget.hxx 2318 2011-12-31 21:56:36Z stephena $
//
//   Based on code from ScummVM - Scumm Interpreter
//   Copyright (C) 2002-2004 The ScummVM project
//============================================================================

#ifndef CPU_WIDGET_HXX
#define CPU_WIDGET_HXX

class GuiObject;
class ButtonWidget;
class DataGridWidget;
class DataGridOpsWidget;
class EditTextWidget;
class ToggleBitWidget;

#include "Widget.hxx"
#include "Command.hxx"

class CpuWidget : public Widget, public CommandSender
{
  public:
    CpuWidget(GuiObject* boss, const GUI::Font& font, int x, int y);
    virtual ~CpuWidget();

    void handleCommand(CommandSender* sender, int cmd, int data, int id);

    void loadConfig();
    void setOpsWidget(DataGridOpsWidget* w);

  private:
    void fillGrid();

  private:
    // ID's for the various widgets
    // We need ID's, since there are more than one of several types of widgets
    enum {
      kPCRegID,
      kCpuRegID
    };

    enum {
      kPCRegAddr,
      kSPRegAddr,
      kARegAddr,
      kXRegAddr,
      kYRegAddr
    };

    enum {
      kPSRegN = 0,
      kPSRegV = 1,
      kPSRegB = 3,
      kPSRegD = 4,
      kPSRegI = 5,
      kPSRegZ = 6,
      kPSRegC = 7
    };

    DataGridWidget*  myPCGrid;
    DataGridWidget*  myCpuGrid;
    DataGridWidget*  myCpuGridDecValue;
    DataGridWidget*  myCpuGridBinValue;
    DataGridWidget*  myCpuDataSrcGrid;
    ToggleBitWidget* myPSRegister;
    EditTextWidget*  myPCLabel;
};

#endif
