#include "rsxdebugfontrenderer.h"

#include "vpshader_dbgfont_vpo.h"
#include "fpshader_dbgfont_fpo.h"

u8* RSXDebugFontRenderer::spTextureData;

gcmContextData* RSXDebugFontRenderer::mContext = NULL;

u8* RSXDebugFontRenderer::mpTexture = NULL;
u8* RSXDebugFontRenderer::mPosition = NULL;
u8* RSXDebugFontRenderer::mTexCoord = NULL;
u8* RSXDebugFontRenderer::mColor = NULL;

rsxProgramAttrib* RSXDebugFontRenderer::mPosIndex = NULL;
rsxProgramAttrib* RSXDebugFontRenderer::mTexIndex = NULL;
rsxProgramAttrib* RSXDebugFontRenderer::mColIndex = NULL;
rsxProgramAttrib* RSXDebugFontRenderer::mTexUnit = NULL;

rsxVertexProgram* RSXDebugFontRenderer::mRSXVertexProgram;
rsxFragmentProgram* RSXDebugFontRenderer::mRSXFragmentProgram;

void* RSXDebugFontRenderer::mVertexProgramUCode;
void* RSXDebugFontRenderer::mFragmentProgramUCode;

vu32* RSXDebugFontRenderer::mLabel = NULL;
u32 RSXDebugFontRenderer::mLabelValue = 0;

u32 RSXDebugFontRenderer::mFragmentProgramOffset;
u32 RSXDebugFontRenderer::mTextureOffset;
u32 RSXDebugFontRenderer::mPositionOffset;
u32 RSXDebugFontRenderer::mTexCoordOffset;
u32 RSXDebugFontRenderer::mColorOffset;

RSXDebugFontRenderer::RSXDebugFontRenderer() : DebugFontRenderer()
{

}

RSXDebugFontRenderer::RSXDebugFontRenderer(gcmContextData *context) : DebugFontRenderer()
{
	mContext = context;
}

RSXDebugFontRenderer::~RSXDebugFontRenderer()
{

}

void RSXDebugFontRenderer::initShader()
{
	mRSXVertexProgram = (rsxVertexProgram*)vpshader_dbgfont_vpo;
	mRSXFragmentProgram = (rsxFragmentProgram*)fpshader_dbgfont_fpo;

	void *ucode;
	u32 ucodeSize;

	rsxFragmentProgramGetUCode(mRSXFragmentProgram, &ucode, &ucodeSize);

	mFragmentProgramUCode = rsxMemalign(64, ucodeSize);
	rsxAddressToOffset(mFragmentProgramUCode, &mFragmentProgramOffset);

	memcpy(mFragmentProgramUCode, ucode, ucodeSize);

	rsxVertexProgramGetUCode(mRSXVertexProgram, &mVertexProgramUCode, &ucodeSize);
}

void RSXDebugFontRenderer::init()
{
	mLabel = (vu32*) gcmGetLabelAddress(sLabelId);
	*mLabel = mLabelValue;

	initShader();

	mPosIndex = rsxVertexProgramGetAttrib(mRSXVertexProgram, "position");
	mTexIndex = rsxVertexProgramGetAttrib(mRSXVertexProgram, "texcoord");
	mColIndex = rsxVertexProgramGetAttrib(mRSXVertexProgram, "color");

	mTexUnit = rsxFragmentProgramGetAttrib(mRSXFragmentProgram, "texture");

	spTextureData = (u8*)rsxMemalign(128, DEBUGFONT_DATA_SIZE);
	mpTexture = (u8*)(intptr_t)rsxAlign(128, (intptr_t)spTextureData);

	u8 *pFontData = (u8*)getFontData();

	for(s32 i=0;i < DEBUGFONT_DATA_SIZE;i++)
		mpTexture[i] = pFontData[i];

	rsxAddressToOffset(mpTexture, &mTextureOffset);

	mPosition = (u8*)rsxMemalign(128, DEBUGFONT_MAX_CHAR_COUNT*NUM_VERTS_PER_GLYPH*sizeof(f32)*3);
	mTexCoord = (u8*)rsxMemalign(128, DEBUGFONT_MAX_CHAR_COUNT*NUM_VERTS_PER_GLYPH*sizeof(f32)*2);
	mColor = (u8*)rsxMemalign(128, DEBUGFONT_MAX_CHAR_COUNT*NUM_VERTS_PER_GLYPH*sizeof(f32)*4);

	rsxAddressToOffset(mPosition, &mPositionOffset);
	rsxAddressToOffset(mTexCoord, &mTexCoordOffset);
	rsxAddressToOffset(mColor, &mColorOffset);
}

void RSXDebugFontRenderer::shutdown()
{

}

void RSXDebugFontRenderer::printStart(f32 r, f32 g, f32 b, f32 a)
{
	sR = r;
	sG = g;
	sB = b;
	sA = a;

	rsxSetBlendFunc(mContext, GCM_SRC_ALPHA, GCM_ONE_MINUS_SRC_ALPHA, GCM_SRC_ALPHA, GCM_ONE_MINUS_SRC_ALPHA);
	rsxSetBlendEquation(mContext, GCM_FUNC_ADD, GCM_FUNC_ADD);
	rsxSetBlendEnable(mContext, GCM_TRUE);
	rsxSetLogicOpEnable(mContext, GCM_FALSE);

	rsxSetDepthTestEnable(mContext, GCM_FALSE);

	rsxLoadVertexProgram(mContext, mRSXVertexProgram, mVertexProgramUCode);
	rsxLoadFragmentProgramLocation(mContext, mRSXFragmentProgram, mFragmentProgramOffset, GCM_LOCATION_RSX);

	gcmTexture tex;
	tex.format = GCM_TEXTURE_FORMAT_B8|GCM_TEXTURE_FORMAT_LIN;
	tex.mipmap = 1;
	tex.dimension = GCM_TEXTURE_DIMS_2D;
	tex.cubemap = GCM_FALSE;
	tex.remap = GCM_TEXTURE_REMAP_TYPE_REMAP<<GCM_TEXTURE_REMAP_TYPE_B_SHIFT |
				GCM_TEXTURE_REMAP_TYPE_REMAP<<GCM_TEXTURE_REMAP_TYPE_G_SHIFT |
				GCM_TEXTURE_REMAP_TYPE_REMAP<<GCM_TEXTURE_REMAP_TYPE_R_SHIFT |
				GCM_TEXTURE_REMAP_TYPE_REMAP<<GCM_TEXTURE_REMAP_TYPE_A_SHIFT |
				GCM_TEXTURE_REMAP_COLOR_B<<GCM_TEXTURE_REMAP_COLOR_B_SHIFT |
				GCM_TEXTURE_REMAP_COLOR_B<<GCM_TEXTURE_REMAP_COLOR_G_SHIFT |
				GCM_TEXTURE_REMAP_COLOR_B<<GCM_TEXTURE_REMAP_COLOR_R_SHIFT |
				GCM_TEXTURE_REMAP_COLOR_B<<GCM_TEXTURE_REMAP_COLOR_A_SHIFT;
	tex.width = DEBUGFONT_TEXTURE_WIDTH;
	tex.height = DEBUGFONT_TEXTURE_HEIGHT;
	tex.depth = 1;
	tex.pitch = DEBUGFONT_TEXTURE_WIDTH;
	tex.location = GCM_LOCATION_RSX;
	tex.offset = mTextureOffset;
	rsxLoadTexture(mContext, mTexUnit->index, &tex);

	rsxTextureControl(mContext, mTexUnit->index, GCM_TRUE, 0<<8, 12<<8, GCM_TEXTURE_MAX_ANISO_1);
	rsxTextureFilter(mContext, mTexUnit->index, 0, GCM_TEXTURE_NEAREST_MIPMAP_LINEAR, GCM_TEXTURE_LINEAR, GCM_TEXTURE_CONVOLUTION_QUINCUNX);
	rsxTextureWrapMode(mContext, mTexUnit->index, GCM_TEXTURE_REPEAT, GCM_TEXTURE_REPEAT, GCM_TEXTURE_REPEAT, GCM_TEXTURE_UNSIGNED_REMAP_NORMAL, GCM_TEXTURE_ZFUNC_LESS, 0);
}

void RSXDebugFontRenderer::printPass(DebugFont::Position *pPositions, DebugFont::TexCoord *pTexCoords, DebugFont::Color *pColors, s32 numVerts)
{
	while(*mLabel != mLabelValue)
		usleep(10);
	mLabelValue++;

	memcpy(mPosition, pPositions, numVerts*sizeof(f32)*3);
	memcpy(mTexCoord, pTexCoords, numVerts*sizeof(f32)*2);
	memcpy(mColor, pColors, numVerts*sizeof(f32)*4);

	rsxBindVertexArrayAttrib(mContext, mPosIndex->index, 0, mPositionOffset, sizeof(f32)*3, 3, GCM_VERTEX_DATA_TYPE_F32, GCM_LOCATION_RSX);
	rsxBindVertexArrayAttrib(mContext, mTexIndex->index, 0, mTexCoordOffset, sizeof(f32)*2, 2, GCM_VERTEX_DATA_TYPE_F32, GCM_LOCATION_RSX);
	rsxBindVertexArrayAttrib(mContext, mColIndex->index, 0, mColorOffset, sizeof(f32)*4, 4, GCM_VERTEX_DATA_TYPE_F32, GCM_LOCATION_RSX);

	rsxDrawVertexArray(mContext, DEBUGFONT_PRIMITIVE, 0, numVerts);
	rsxInvalidateVertexCache(mContext);
	rsxSetWriteBackendLabel(mContext, sLabelId, mLabelValue);

	rsxFlushBuffer(mContext);
}

void RSXDebugFontRenderer::printEnd()
{
	rsxSetDepthTestEnable(mContext, GCM_TRUE);
	rsxSetBlendEnable(mContext, GCM_FALSE);
}
