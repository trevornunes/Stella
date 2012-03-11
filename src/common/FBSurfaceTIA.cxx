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
// $Id: FBSurfaceTIA.cxx 2318 2011-12-31 21:56:36Z stephena $
//============================================================================

#ifdef DISPLAY_OPENGL

#include "Font.hxx"
#include "FrameBufferGL.hxx"
#include "TIA.hxx"

#include "FBSurfaceTIA.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
FBSurfaceTIA::FBSurfaceTIA(FrameBufferGL& buffer, uInt32 baseWidth, uInt32 baseHeight,
                           uInt32 imgX, uInt32 imgY, uInt32 imgW, uInt32 imgH)
  : myFB(buffer),
    myGL(myFB.p_gl),
    myTexture(NULL),
    myVBOID(0),
    myImageX(imgX),
    myImageY(imgY),
    myImageW(imgW),
    myImageH(imgH)
{
  myTexID[0] = myTexID[1] = 0;

  // Fill buffer struct with valid data
  myTexWidth  = FrameBufferGL::power_of_two(baseWidth);
  myTexHeight = FrameBufferGL::power_of_two(baseHeight);
  myTexCoordW = (GLfloat) baseWidth / myTexWidth;
  myTexCoordH = (GLfloat) baseHeight / myTexHeight;

  // Based on experimentation, the following are the fastest 16-bit
  // formats for OpenGL (on all platforms)
  myTexture = SDL_CreateRGBSurface(SDL_SWSURFACE,
                  myTexWidth, myTexHeight, 16,
#ifdef HAVE_GL_BGRA
                  0x00007c00, 0x000003e0, 0x0000001f, 0x00000000);
#else
                  0x0000f800, 0x000007c0, 0x0000003e, 0x00000000);
#endif
  myPitch = myTexture->pitch >> 1;

  // Associate the SDL surface with a GL texture object
  updateCoords();
  reload();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
FBSurfaceTIA::~FBSurfaceTIA()
{
  if(myTexture)
    SDL_FreeSurface(myTexture);

  free();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceTIA::getPos(uInt32& x, uInt32& y) const
{
  x = myImageX;
  y = myImageY;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceTIA::translateCoords(Int32& x, Int32& y) const
{
  x -= myImageX;
  y -= myImageY;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceTIA::update()
{
  // Copy the mediasource framebuffer to the RGB texture
  // In OpenGL mode, it's faster to just assume that the screen is dirty
  // and always do an update

  uInt8* currentFrame  = myTIA->currentFrameBuffer();
  uInt8* previousFrame = myTIA->previousFrameBuffer();
  uInt32 width         = myTIA->width();
  uInt32 height        = myTIA->height();
  uInt16* buffer       = (uInt16*) myTexture->pixels;

  if(!myFB.myUsePhosphor)
  {
    uInt32 bufofsY    = 0;
    uInt32 screenofsY = 0;
    for(uInt32 y = 0; y < height; ++y )
    {
      uInt32 pos = screenofsY;
      for(uInt32 x = 0; x < width; ++x )
        buffer[pos++] = (uInt16) myFB.myDefPalette[currentFrame[bufofsY + x]];

      bufofsY    += width;
      screenofsY += myPitch;
    }
  }
  else
  {
    uInt32 bufofsY    = 0;
    uInt32 screenofsY = 0;
    for(uInt32 y = 0; y < height; ++y )
    {
      uInt32 pos = screenofsY;
      for(uInt32 x = 0; x < width; ++x )
      {
        const uInt32 bufofs = bufofsY + x;
        buffer[pos++] = (uInt16)
          myFB.myAvgPalette[currentFrame[bufofs]][previousFrame[bufofs]];
      }
      bufofsY    += width;
      screenofsY += myPitch;
    }
  }

  // Texturemap complete texture to surface so we have free scaling
  // and antialiasing
  myGL.ActiveTexture(GL_TEXTURE0);
  myGL.BindTexture(GL_TEXTURE_2D, myTexID[0]);
  myGL.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, myTexWidth, myTexHeight,
#ifdef HAVE_GL_BGRA
                    GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV,
#else
                    GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1,
#endif
                    myTexture->pixels);

  myGL.EnableClientState(GL_VERTEX_ARRAY);
  myGL.EnableClientState(GL_TEXTURE_COORD_ARRAY);

  if(myFB.myVBOAvailable)
  {
    myGL.BindBuffer(GL_ARRAY_BUFFER, myVBOID);
    myGL.VertexPointer(2, GL_FLOAT, 0, (const GLvoid*)0);
    myGL.TexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*)(8*sizeof(GLfloat)));
    myGL.DrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#if 0
    if(1)//myFB.myScanlinesEnabled)
    {
      myGL.Enable(GL_BLEND);
      myGL.Color4f(1.0f, 1.0f, 1.0f, 0.5f);
      myGL.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      myGL.BindTexture(GL_TEXTURE_2D, myTexID[1]);
      myGL.VertexPointer(2, GL_FLOAT, 0, (const GLvoid*)(16*sizeof(GLfloat)));
      myGL.TexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*)(24*sizeof(GLfloat)));
      myGL.DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      myGL.Disable(GL_BLEND);
    }
#endif
  }
  else
  {
    myGL.VertexPointer(2, GL_FLOAT, 0, myCoord);
    myGL.TexCoordPointer(2, GL_FLOAT, 0, myCoord+8);
    myGL.DrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#if 0
    if(1)//myFB.myScanlinesEnabled)
    {
      myGL.Enable(GL_BLEND);
      myGL.Color4f(1.0f, 1.0f, 1.0f, 0.5f);
      myGL.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      myGL.BindTexture(GL_TEXTURE_2D, myTexID[1]);
      myGL.VertexPointer(2, GL_FLOAT, 0, myCoord+16);
      myGL.TexCoordPointer(2, GL_FLOAT, 0, myCoord+24);
      myGL.DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      myGL.Disable(GL_BLEND);
    }
#endif
  }

  myGL.DisableClientState(GL_VERTEX_ARRAY);
  myGL.DisableClientState(GL_TEXTURE_COORD_ARRAY);

  // Let postFrameUpdate() know that a change has been made
  myFB.myDirtyFlag = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceTIA::free()
{
  myGL.DeleteTextures(2, myTexID);
  if(myFB.myVBOAvailable)
    myGL.DeleteBuffers(1, &myVBOID);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceTIA::reload()
{
  // This does a 'soft' reset of the surface
  // It seems that on some system (notably, OSX), creating a new SDL window
  // destroys the GL context, requiring a reload of all textures
  // However, destroying the entire FBSurfaceGL object is wasteful, since
  // it will also regenerate SDL software surfaces (which are not required
  // to be regenerated)
  // Basically, all that needs to be done is to re-call glTexImage2D with a
  // new texture ID, so that's what we do here

  myGL.ActiveTexture(GL_TEXTURE0);
  myGL.Enable(GL_TEXTURE_2D);

  // TIA surfaces also use a scanline texture
  myGL.GenTextures(2, myTexID);

  // Base texture (@ index 0)
  myGL.BindTexture(GL_TEXTURE_2D, myTexID[0]);
  myGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  myGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  myGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  myGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // Create the texture in the most optimal format
  myGL.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 myTexWidth, myTexHeight, 0,
#ifdef HAVE_GL_BGRA
                 GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV,
#else
                 GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1,
#endif
                 myTexture->pixels);

  // Scanline texture (@ index 1)
  myGL.BindTexture(GL_TEXTURE_2D, myTexID[1]);
  myGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  myGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  myGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  myGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#ifdef HAVE_GL_BGRA
  static uInt16 const scanline[4] = { 0x0000, 0x0000, 0x8000, 0x0000  };
  myGL.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 2, 0,
                 GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV,
                 scanline);
#else
  static uInt16 const scanline[4] = { 0x0000, 0x0000, 0x0001, 0x0000 };
  myGL.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 2, 0,
                 GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1,
                 scanline);
#endif

  // Cache vertex and texture coordinates using vertex buffer object
  if(myFB.myVBOAvailable)
  {
    myGL.GenBuffers(1, &myVBOID);
    myGL.BindBuffer(GL_ARRAY_BUFFER, myVBOID);
    myGL.BufferData(GL_ARRAY_BUFFER, 32*sizeof(GLfloat), myCoord, GL_STATIC_DRAW);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceTIA::setFilter(const string& name)
{
  // We only do GL_NEAREST or GL_LINEAR for now
  GLint filter = GL_NEAREST;
  if(name == "linear")
    filter = GL_LINEAR;

  myGL.BindTexture(GL_TEXTURE_2D, myTexID[0]);
  myGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
  myGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
  myGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  myGL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceTIA::updateCoords()
{
  // Vertex coordinates for texture 0 (main texture)
  // Upper left (x,y)
  myCoord[0] = (GLfloat)myImageX;
  myCoord[1] = (GLfloat)myImageY;
  // Upper right (x+w,y)
  myCoord[2] = (GLfloat)(myImageX + myImageW);
  myCoord[3] = (GLfloat)myImageY;
  // Lower left (x,y+h)
  myCoord[4] = (GLfloat)myImageX;
  myCoord[5] = (GLfloat)(myImageY + myImageH);
  // Lower right (x+w,y+h)
  myCoord[6] = (GLfloat)(myImageX + myImageW);
  myCoord[7] = (GLfloat)(myImageY + myImageH);

  // Texture coordinates for texture 0 (main texture)
  // Upper left (x,y)
  myCoord[8] = 0.0f;
  myCoord[9] = 0.0f;
  // Upper right (x+w,y)
  myCoord[10] = myTexCoordW;
  myCoord[11] = 0.0f;
  // Lower left (x,y+h)
  myCoord[12] = 0.0f;
  myCoord[13] = myTexCoordH;
  // Lower right (x+w,y+h)
  myCoord[14] = myTexCoordW;
  myCoord[15] = myTexCoordH;

  // Vertex coordinates for texture 1 (scanline texture)
  // Upper left (x,y)
  myCoord[16] = (GLfloat)myImageX;
  myCoord[17] = (GLfloat)myImageY;
  // Upper right (x+w,y)
  myCoord[18] = (GLfloat)(myImageX + myImageW);
  myCoord[19] = (GLfloat)myImageY;
  // Lower left (x,y+h)
  myCoord[20] = (GLfloat)myImageX;
  myCoord[21] = (GLfloat)(myImageY + myImageH);
  // Lower right (x+w,y+h)
  myCoord[22] = (GLfloat)(myImageX + myImageW);
  myCoord[23] = (GLfloat)(myImageY + myImageH);

  // Texture coordinates for texture 1 (scanline texture)
  // Upper left (x,y)
  myCoord[24] = 0.0f;
  myCoord[25] = 0.0f;
  // Upper right (x+w,y)
  myCoord[26] = 1.0f;
  myCoord[27] = 0.0f;
  // Lower left (x,y+h)
  myCoord[28] = 0.0f;
  myCoord[29] = (GLfloat)(myImageH/myFB.myZoomLevel);
  // Lower right (x+w,y+h)
  myCoord[30] = 1.0f;
  myCoord[31] = (GLfloat)(myImageH/myFB.myZoomLevel);

  // Cache vertex and texture coordinates using vertex buffer object
  if(myFB.myVBOAvailable)
  {
    myGL.BindBuffer(GL_ARRAY_BUFFER, myVBOID);
    myGL.BufferData(GL_ARRAY_BUFFER, 32*sizeof(GLfloat), myCoord, GL_STATIC_DRAW);
  }
}

#endif
