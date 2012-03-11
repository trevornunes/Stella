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
// $Id: ContextMenu.hxx 2356 2012-01-14 22:00:54Z stephena $
//
//   Based on code from ScummVM - Scumm Interpreter
//   Copyright (C) 2002-2004 The ScummVM project
//============================================================================

#ifndef CONTEXT_MENU_HXX
#define CONTEXT_MENU_HXX

#include "bspf.hxx"
#include "Command.hxx"
#include "Dialog.hxx"

enum {
  kCMenuItemSelectedCmd = 'CMsl'
};

/**
 * Popup context menu which, when clicked, "pop up" a list of items and
 * lets the user pick on of them.
 *
 * Implementation wise, when the user selects an item, then the given 'cmd'
 * is broadcast, with data being equal to the tag value of the selected entry.
 */
class ContextMenu : public Dialog, public CommandSender
{
  public:
    ContextMenu(GuiObject* boss, const GUI::Font& font,
                const StringMap& items, int cmd = 0);
    virtual ~ContextMenu();

    /** Add the given items to the widget. */
    void addItems(const StringMap& items);

    /** Show context menu onscreen at the specified coordinates */
    void show(uInt32 x, uInt32 y, int item = -1);

    /** Select the entry at the given index. */
    void setSelected(int item);
	
    /** Select the first entry matching the given tag. */
    void setSelected(const string& tag, const string& defaultTag);

    /** Select the highest/last entry in the internal list. */
    void setSelectedMax();

    /** Clear selection (reset to default). */
    void clearSelection();

    /** Accessor methods for the currently selected item. */
    int getSelected() const;
    const string& getSelectedName() const;
    const string& getSelectedTag() const;

    /** This dialog uses its own positioning, so we override Dialog::center() */
    void center();

  protected:
    void handleMouseDown(int x, int y, int button, int clickCount);
    void handleMouseMoved(int x, int y, int button);
    bool handleMouseClicks(int x, int y, int button);
    void handleMouseWheel(int x, int y, int direction);
    void handleKeyDown(StellaKey key, StellaMod mod, char ascii);
    void handleJoyDown(int stick, int button);
    void handleJoyAxis(int stick, int axis, int value);
    bool handleJoyHat(int stick, int hat, int value);
    void handleEvent(Event::Type e);

    void drawDialog();

  private:
    void recalc(const GUI::Rect& image);
	
    int findItem(int x, int y) const;
    void drawCurrentSelection(int item);
	
    void moveUp();
    void moveDown();
    void scrollUp(int distance = 1);
    void scrollDown(int distance = 1);
    void sendSelection();

  private:
    StringMap _entries;

    int _rowHeight;
    int _firstEntry, _numEntries;
    int _selectedOffset, _selectedItem;
    bool _showScroll;
    bool _isScrolling;
    uInt32 _scrollUpColor, _scrollDnColor;

    const GUI::Font* _font;
    int _cmd;

    uInt32 _xorig, _yorig;
};

#endif
