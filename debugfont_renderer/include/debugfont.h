#ifndef __DEBUGFONT_H__
#define __DEBUGFONT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <math.h>
#include <setjmp.h>
#include <ppu-asm.h>
#include <ppu-types.h>

#define DEBUGFONT_TEXTURE_WIDTH				128
#define DEBUGFONT_TEXTURE_HEIGHT			128
#define DEBUGFONT_GLYPH_WIDTH				8
#define DEBUGFONT_GLYPH_HEIGHT				9

#define DEBUGFONT_DATA_SIZE					(DEBUGFONT_TEXTURE_WIDTH*DEBUGFONT_TEXTURE_HEIGHT)
#define DEBUGFONT_TAB_SIZE					4
#define DEBUGFONT_MAX_CHAR_COUNT			256
#define DEBUGFONT_DEFAULT_SAFE_AREA			40

#define DEBUGFONT_USE_QUADS

class DebugFontRenderer;

class DebugFont
{
    friend class DebugFontRenderer;

public:
    static void init();
    static void shutdown();

	static inline void setPosition(s32 x, s32 y)
	{
		sXPos = x;
		sYPos = y;
	}

	static inline void setColor(f32 r, f32 g, f32 b, f32 a)
	{
		sR = r;
		sG = g;
		sB = b;
		sA = a;
	}

	static void print(const char *pszText);
	static void printf(const char *pszText, ...);

	static inline s32 getGlyphWidth()
	{
		return DEBUGFONT_GLYPH_WIDTH;
	}

	static inline s32 getGlyphHeight()
	{
		return DEBUGFONT_GLYPH_HEIGHT;
	}

	static void getExtents(const char *pszText, s32 *pWidth, s32 *pHeight, s32 srcWidth);

	static inline void setScreenRes(s32 x, s32 y)
	{
		sXRes = x;
		sYRes = y;
	}

	static inline void setSafeArea(s32 left, s32 right, s32 top, s32 bottom)
	{
		sLeftSafe = left;
		sRightSafe = right;
		sTopSafe = top;
		sBottomSafe = bottom;
	}

	static inline void getSafeArea(s32 *pLeft, s32 *pRight, s32 *pTop, s32 *pBottom)
	{
		*pLeft = sLeftSafe;
		*pRight = sRightSafe;
		*pTop = sTopSafe;
		*pBottom = sBottomSafe;
	}

	struct Position
	{
		f32 x, y, z;
	};

	struct TexCoord
	{
		f32 s, t;
	};

	struct Color
	{
		f32 r, g, b, a;
	};

private:
	static void print(const char *pszText, s32 count);

	static inline f32 calcPos(s32 pos, s32 dim)
	{
		return ((f32)pos - ((f32)dim*0.5f))/((f32)dim*0.5f);
	}

	static inline f32 calcS0(u8 c)
	{
		return (f32)((c%16)*getGlyphWidth())/(f32)DEBUGFONT_TEXTURE_WIDTH;
	}

	static inline f32 calcS1(u8 c)
	{
		return (f32)(((c%16)*getGlyphWidth()) + getGlyphWidth())/(f32)DEBUGFONT_TEXTURE_WIDTH;
	}

	static inline f32 calcT0(u8 c)
	{
		return (f32)((((c/16) - 2)*getGlyphHeight()) + 1)/(f32)DEBUGFONT_TEXTURE_HEIGHT;
	}

	static inline f32 calcT1(u8 c)
	{
		return (f32)((((c/16) - 2)*getGlyphHeight()) + getGlyphHeight() + 1)/(f32)DEBUGFONT_TEXTURE_HEIGHT;
	}

	static inline bool isPrintable(char c)
	{
		return ((u8)c&0x7f) > 31;
	}

	static u8 sFontData[DEBUGFONT_DATA_SIZE];
	static s32 sXPos;
	static s32 sYPos;
	static s32 sXRes;
	static s32 sYRes;
	static s32 sLeftSafe;
	static s32 sRightSafe;
	static s32 sTopSafe;
	static s32 sBottomSafe;
	static f32 sR;
	static f32 sG;
	static f32 sB;
	static f32 sA;

	static Position *spPositions[2];
	static TexCoord *spTexCoords[2];
	static Color *spColors[2];

	static Color sDefaultColors[8];

	static DebugFontRenderer *spRenderer;
};

#endif // __DEBUGFONT_H__
