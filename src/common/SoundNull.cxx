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
// $Id: SoundNull.cxx 2318 2011-12-31 21:56:36Z stephena $
//============================================================================

#include "bspf.hxx"

#include "OSystem.hxx"
#include "Settings.hxx"
#include "SoundNull.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SoundNull::SoundNull(OSystem* osystem)
  : Sound(osystem)
{
  myOSystem->logMessage("Sound disabled.\n\n", 1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SoundNull::~SoundNull()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool SoundNull::load(Serializer& in)
{
  if(in.getString() != "TIASound")
    return false;

  // Read sound registers and discard
  in.getByte();
  in.getByte();
  in.getByte();
  in.getByte();
  in.getByte();
  in.getByte();

  // myLastRegisterSetCycle
  in.getInt();

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool SoundNull::save(Serializer& out) const
{
  out.putString("TIASound");

  out.putByte(0);
  out.putByte(0);
  out.putByte(0);
  out.putByte(0);
  out.putByte(0);
  out.putByte(0);

  // myLastRegisterSetCycle
  out.putInt(0);

  return true;
}
