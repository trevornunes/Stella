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
// $Id: BrowserDialog.hxx 2318 2011-12-31 21:56:36Z stephena $
//
//   Based on code from ScummVM - Scumm Interpreter
//   Copyright (C) 2002-2004 The ScummVM project
//============================================================================

#ifndef BROWSER_DIALOG_HXX
#define BROWSER_DIALOG_HXX

class GuiObject;
class ButtonWidget;
class StaticTextWidget;
class StringListWidget;
class GameList;

#include "Dialog.hxx"
#include "Command.hxx"
#include "FSNode.hxx"
#include "bspf.hxx"

class BrowserDialog : public Dialog, public CommandSender
{
  public:
    BrowserDialog(GuiObject* boss, const GUI::Font& font, int max_w, int max_h);
    virtual ~BrowserDialog();

    const FilesystemNode& getResult() { return _node; }

    /** Place the browser window onscreen, using the given attributes */
    void show(const string& title, const string& startpath,
              FilesystemNode::ListMode mode, int cmd);

  protected:
    virtual void handleCommand(CommandSender* sender, int cmd, int data, int id);
    void updateListing();

  private:
    enum {
      kChooseCmd  = 'CHOS',
      kGoUpCmd    = 'GOUP',
      kBaseDirCmd = 'BADR'
    };

    int	_cmd;

    StringListWidget* _fileList;
    StaticTextWidget* _currentPath;
    StaticTextWidget* _title;
    ButtonWidget*     _goUpButton;
    ButtonWidget*     _basedirButton;

    FilesystemNode _node;
    GameList*      _nodeList;

    FilesystemNode::ListMode _mode;
};

#endif
