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
// $Id: Snapshot.hxx 2318 2011-12-31 21:56:36Z stephena $
//============================================================================

#ifndef SNAPSHOT_HXX
#define SNAPSHOT_HXX

class Properties;
class FrameBuffer;
class TIA;

#include <fstream>
#include "bspf.hxx"

class Snapshot
{
  public:
    /**
      Save the current TIA image to a PNG file using data from the Framebuffer.
      Any postprocessing/filtering will be included.

      @param framebuffer The framebuffer containing the image data
      @param props       The properties object containing info about the ROM
      @param filename    The filename of the PNG file
    */
    static string savePNG(const FrameBuffer& framebuffer, const Properties& props,
                          const string& filename);

    /**
      Save the current TIA image to a PNG file using data directly from
      the TIA framebuffer.  No filtering or scaling will be included.

      @param framebuffer The framebuffer containing the image data
      @param mediasrc    Source of the raw TIA data
      @param props       The properties object containing info about the ROM
      @param filename    The filename of the PNG file
    */
    static string savePNG(const FrameBuffer& framebuffer, const TIA& tia,
                          const Properties& props, const string& filename);

  private:
    static string saveBufferToPNG(ofstream& out, uInt8* buffer,
                                  uInt32 width, uInt32 height,
                                  const Properties& props);
    static void writePNGChunk(ofstream& out, const char* type, uInt8* data, int size);
    static void writePNGText(ofstream& out, const string& key, const string& text);
};

#endif
