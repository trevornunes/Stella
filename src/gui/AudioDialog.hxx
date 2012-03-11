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
// $Id: AudioDialog.hxx 2318 2011-12-31 21:56:36Z stephena $
//
//   Based on code from ScummVM - Scumm Interpreter
//   Copyright (C) 2002-2004 The ScummVM project
//============================================================================

#ifndef AUDIO_DIALOG_HXX
#define AUDIO_DIALOG_HXX

class CommandSender;
class Dialog;
class DialogContainer;
class PopUpWidget;
class SliderWidget;
class StaticTextWidget;
class CheckboxWidget;
class OSystem;

#include "bspf.hxx"

class AudioDialog : public Dialog
{
  public:
    AudioDialog(OSystem* osystem, DialogContainer* parent, const GUI::Font& font);
    ~AudioDialog();

  protected:
    SliderWidget*     myVolumeSlider;
    StaticTextWidget* myVolumeLabel;
    PopUpWidget*      myFragsizePopup;
    PopUpWidget*      myFreqPopup;
    PopUpWidget*      myTiaFreqPopup;
    CheckboxWidget*   myClipVolumeCheckbox;
    CheckboxWidget*   mySoundEnableCheckbox;

  private:
    void loadConfig();
    void saveConfig();
    void setDefaults();

    void handleSoundEnableChange(bool active);
    virtual void handleCommand(CommandSender* sender, int cmd, int data, int id);

    enum {
      kVolumeChanged      = 'ADvc',
      kSoundEnableChanged = 'ADse'
    };
};

#endif
