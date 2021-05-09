#include <debugfont.h>
#include <debugfontrenderer.h>

#ifdef DEBUGFONT_USE_QUADS
#define NUM_VERTS_PER_GLYPH				4
#else
#define NUM_VERTS_PER_GLYPH				6
#endif

u8 DebugFont::sFontData[DEBUGFONT_DATA_SIZE] =
{
	#include "debugfontdata.h"
};

s32 DebugFont::sXPos;
s32 DebugFont::sYPos;

s32 DebugFont::sXRes;
s32 DebugFont::sYRes;

s32 DebugFont::sLeftSafe;
s32 DebugFont::sRightSafe;
s32 DebugFont::sTopSafe;
s32 DebugFont::sBottomSafe;

f32 DebugFont::sR;
f32 DebugFont::sG;
f32 DebugFont::sB;
f32 DebugFont::sA;

DebugFont::Position *DebugFont::spPositions[2];
DebugFont::TexCoord *DebugFont::spTexCoords[2];
DebugFont::Color *DebugFont::spColors[2];

DebugFont::Color DebugFont::sDefaultColors[8] =
{
	{ 0.f, 0.f, 0.f, 1.f },	// black
	{ 0.f, 0.f, 1.f, 1.f },	// blue
	{ 1.f, 0.f, 0.f, 1.f },	// red
	{ 1.f, 0.f, 1.f, 1.f },	// magenta
	{ 0.f, 1.f, 0.f, 1.f },	// green
	{ 0.f, 1.f, 1.f, 1.f },	// cyan
	{ 1.f, 1.f, 0.f, 1.f },	// yellow
	{ 1.f, 1.f, 1.f, 1.f },	// white
};

DebugFontRenderer *DebugFont::spRenderer = NULL;

void DebugFont::init()
{
	if(!spRenderer) return;

	spRenderer->init();

	sXPos = sYPos = 0;
	sLeftSafe = sRightSafe = sTopSafe = sBottomSafe = DEBUGFONT_DEFAULT_SAFE_AREA;
	sR = sG = sB = 1.0f;

	spPositions[0] = new Position[DEBUGFONT_MAX_CHAR_COUNT*NUM_VERTS_PER_GLYPH];
	spPositions[1] = new Position[DEBUGFONT_MAX_CHAR_COUNT*NUM_VERTS_PER_GLYPH];
	spTexCoords[0] = new TexCoord[DEBUGFONT_MAX_CHAR_COUNT*NUM_VERTS_PER_GLYPH];
	spTexCoords[1] = new TexCoord[DEBUGFONT_MAX_CHAR_COUNT*NUM_VERTS_PER_GLYPH];
	spColors[0] = new Color[DEBUGFONT_MAX_CHAR_COUNT*NUM_VERTS_PER_GLYPH];
	spColors[1] = new Color[DEBUGFONT_MAX_CHAR_COUNT*NUM_VERTS_PER_GLYPH];
}

void DebugFont::shutdown()
{
	if(!spRenderer) return;

	spRenderer->shutdown();

	delete [] spPositions[0];
	delete [] spPositions[1];
	delete [] spTexCoords[0];
	delete [] spTexCoords[1];
	delete [] spColors[0];
	delete [] spColors[1];
}

void DebugFont::printf(const char *pszText, ...)
{
	va_list argList;
	char tempStr[1024];

	va_start(argList, pszText);
	vsprintf(tempStr, pszText, argList);
	va_end(argList);

	print(tempStr, strlen(tempStr));
}

void DebugFont::print(const char *pszText)
{
	print(pszText, strlen(pszText));
}

void DebugFont::print(const char *pszText, s32 count)
{
	s32 c, offset, pass, numPasses;

	if(!spRenderer) return;

	offset = 0;
	numPasses = (count/DEBUGFONT_MAX_CHAR_COUNT) + 1;

	spRenderer->printStart(sR, sG, sB, sA);

	for(pass=0;pass < numPasses;pass++) {
		s32 numVerts = 0;
		TexCoord *pTexCoords = spTexCoords[pass&1];
		Position *pPositions = spPositions[pass&1];
		Color *pColors = spColors[pass&1];

		for(c=0;(offset + c) < count && c < DEBUGFONT_MAX_CHAR_COUNT;c++) {
			if(isPrintable(pszText[offset + c])) {
				// top left
				pPositions[numVerts].x = calcPos(sXPos + sLeftSafe, sXRes);
				pPositions[numVerts].y = calcPos(sYPos + sTopSafe, sYRes)*-1.0f;
				pPositions[numVerts].z = 0.0f;
				pTexCoords[numVerts].s = calcS0((u8)pszText[offset + c]);
				pTexCoords[numVerts].t = calcT0((u8)pszText[offset + c]);
				pColors[numVerts].r = sR;
				pColors[numVerts].g = sG;
				pColors[numVerts].b = sB;
				pColors[numVerts].a = sA;
				numVerts++;

				// top right
				pPositions[numVerts].x = calcPos(sXPos + sLeftSafe + getGlyphWidth(), sXRes);
				pPositions[numVerts].y = calcPos(sYPos + sTopSafe, sYRes)*-1.0f;
				pPositions[numVerts].z = 0.0f;
				pTexCoords[numVerts].s = calcS1((u8)pszText[offset + c]);
				pTexCoords[numVerts].t = calcT0((u8)pszText[offset + c]);
				pColors[numVerts].r = sR;
				pColors[numVerts].g = sG;
				pColors[numVerts].b = sB;
				pColors[numVerts].a = sA;
				numVerts++;

				// bottom right
				pPositions[numVerts].x = calcPos(sXPos + sLeftSafe + getGlyphWidth(), sXRes);
				pPositions[numVerts].y = calcPos(sYPos + sTopSafe + getGlyphHeight(), sYRes)*-1.0f;
				pPositions[numVerts].z = 0.0f;
				pTexCoords[numVerts].s = calcS1((u8)pszText[offset + c]);
				pTexCoords[numVerts].t = calcT1((u8)pszText[offset + c]);
				pColors[numVerts].r = sR;
				pColors[numVerts].g = sG;
				pColors[numVerts].b = sB;
				pColors[numVerts].a = sA;
				numVerts++;
#ifndef DEBUGFONT_USE_QUADS
				// bottom right
				pPositions[numVerts].x = calcPos(sXPos + sLeftSafe + getGlyphWidth(), sXRes);
				pPositions[numVerts].y = calcPos(sYPos + sTopSafe + getGlyphHeight(), sYRes)*-1.0f;
				pPositions[numVerts].z = 0.0f;
				pTexCoords[numVerts].s = calcS1((u8)pszText[offset + c]);
				pTexCoords[numVerts].t = calcT1((u8)pszText[offset + c]);
				pColors[numVerts].r = sR;
				pColors[numVerts].g = sG;
				pColors[numVerts].b = sB;
				pColors[numVerts].a = sA;
				numVerts++;
#endif
				// bottom left
				pPositions[numVerts].x = calcPos(sXPos + sLeftSafe, sXRes);
				pPositions[numVerts].y = calcPos(sYPos + sTopSafe + getGlyphHeight(), sYRes)*-1.0f;
				pPositions[numVerts].z = 0.0f;
				pTexCoords[numVerts].s = calcS0((u8)pszText[offset + c]);
				pTexCoords[numVerts].t = calcT1((u8)pszText[offset + c]);
				pColors[numVerts].r = sR;
				pColors[numVerts].g = sG;
				pColors[numVerts].b = sB;
				pColors[numVerts].a = sA;
				numVerts++;
#ifndef DEBUGFONT_USE_QUADS
				// top left
				pPositions[numVerts].x = calcPos(sXPos + sLeftSafe, sXRes);
				pPositions[numVerts].y = calcPos(sYPos + sTopSafe, sYRes)*-1.0f;
				pPositions[numVerts].z = 0.0f;
				pTexCoords[numVerts].s = calcS0((u8)pszText[offset + c]);
				pTexCoords[numVerts].t = calcT0((u8)pszText[offset + c]);
				pColors[numVerts].r = sR;
				pColors[numVerts].g = sG;
				pColors[numVerts].b = sB;
				pColors[numVerts].a = sA;
				numVerts++;
#endif
				sXPos += (getGlyphWidth() + 1);
			}
			else if(pszText[offset + c] == '\n') {
				sXPos = 0;
				sYPos += (getGlyphHeight() + 1);
			}
			else if(pszText[offset + c] == '\t') {
				sXPos += DEBUGFONT_TAB_SIZE*(getGlyphWidth() + 1);
			}
			else if((s8)pszText[offset + c] < 0) {
				s32 iCode = (u8)pszText[offset + c] - 128;
				if(iCode >= 0 && iCode < 8) {
					sR = sDefaultColors[iCode].r;
					sG = sDefaultColors[iCode].g;
					sB = sDefaultColors[iCode].b;
				}
			}
			else {
				sXPos += (getGlyphWidth() + 1);
			}

			if(sXPos + (getGlyphWidth() + 1) >= sXRes - (sLeftSafe + sRightSafe)) {
				sXPos = 0;
				sYPos += (getGlyphHeight() + 1);
			}
		}

		offset += DEBUGFONT_MAX_CHAR_COUNT;

		spRenderer->printPass(pPositions, pTexCoords, pColors, numVerts);
	}

	spRenderer->printEnd();
}

void DebugFont::getExtents(const char *pszText, s32 *pWidth, s32 *pHeight, s32 srcWidth)
{
	s32 len, x, y;
	bool lineFeed = true;

	x = y = 0;
	len = strlen(pszText);
	*pWidth = *pHeight = 0;

	for(s32 i=0;i < len;i++) {
		if(isPrintable(pszText[i])) {
			x += (getGlyphWidth() + 1);
			if(lineFeed) {
				lineFeed = false;
				y += (getGlyphHeight() + 1);
			}
		}
		else if(pszText[i] == '\n') {
			x = 0;
			lineFeed = true;
		}
		else if(pszText[i] == '\t') {
			x += DEBUGFONT_TAB_SIZE*(getGlyphWidth() + 1);
		}
		else if((s8)pszText[i] < 0) {}
		else {
			x += (getGlyphWidth() + 1);
		}

		if(x + (getGlyphWidth() + 1) >= srcWidth - (sLeftSafe + sRightSafe)) {
			x = 0;
			lineFeed = true;
		}

		if(x > *pWidth) *pWidth = x;
		if(y > *pHeight) *pHeight = y;
	}
}
