
#include <iostream>
#include <stdio.h>
#include <PSGL/psgl.h>
#include <PSGL/psglu.h>
#include <sys/spu_initialize.h>
#include <cell/sysmodule.h>
#include <cell/dbgfont.h>
#include <sys/timer.h>

#define COLOR_ORANGE	0xff1780f8
#define COLOR_GREEN		0xff00ff00
#define COLOR_YELLOW	0xff00ccff
#define COLOR_RED		0xff0000ff
#define COLOR_WHITE		0xffffffff
#define COLOR_BLACK		0xff000000

// Piece of code about Ps3MapiDyn
#define DYN_SIZE(x)  DYN256B(x) // DYN64K(DYN24B(DYN8B(x))) -> 16384 nop * 6 nop * 2 nop = 786432 Bytes (plus overhead)
#include "ps3mapidyn.h"

// Let's create the byte code about a simply function: int returnFive() {return 5;}
// This is the assembly:
// li		r3,0x05
// blr 
// fret5 array will contain the bytecode of returnFive
char fret5[32] = { 0x38, 0x60, 0x00, 0x05, 0x4E, 0x80, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// fret9 will contain the bytecode of returnNine() {return 9;}		  
char fret9[32] = { 0x38, 0x60, 0x00, 0x09, 0x4E, 0x80, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// I added 8 zero bytes because i noticed that ps3mapi_set_process_mem works correctly with lengths 16 byte aligned
// //////////// //

struct PSGLdevice* device;
struct PSGLcontext* context;

GLuint					renderWidth, renderHeight;
unsigned int			deviceWidth, deviceHeight;

unsigned int frames;

void initGraphics()
{
	
	// clear the screen on startup
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	psglSwap();

}

void onRender()
{
	psglGetRenderBufferDimensions(device, &renderWidth, &renderHeight);

	glViewport(0, 0, renderWidth, renderHeight);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cellDbgFontDraw();
	psglSwap();
}

void onShutdown()
{
	if (context) psglDestroyContext(context);
	if (device) psglDestroyDevice(device);
	sys_timer_usleep(100000);
	cellDbgFontExit();
	sys_timer_usleep(100000);
	psglExit();
	sys_timer_usleep(100000);
}

void dbgFontInit()
{
	int ret;

	CellDbgFontConfig cfg;
	memset(&cfg, 0, sizeof(CellDbgFontConfig));

	cfg.bufSize = 1024 * 10;
	cfg.screenWidth = deviceWidth;
	cfg.screenHeight = deviceHeight;

	ret = cellDbgFontInit(&cfg);

	if (ret != CELL_OK)
	{
		// error...
		return;
	}

}

static int chooseBestResolution(const uint32_t *resolutions, uint32_t numResolutions)
{
	uint32_t bestResolution = 0;

	for (uint32_t i = 0; bestResolution == 0 && i < numResolutions; i++) 
	{
		if(cellVideoOutGetResolutionAvailability(CELL_VIDEO_OUT_PRIMARY, resolutions[i], CELL_VIDEO_OUT_ASPECT_AUTO, 0))
		{
			bestResolution = resolutions[i];
		}
	}
	return bestResolution;
}

static int getResolutionWidthHeight(const uint32_t resolutionId, uint32_t &w, uint32_t &h)
{
	switch(resolutionId)
	{
		case CELL_VIDEO_OUT_RESOLUTION_480: 
			w=720;  h=480;  
			return 1;
		case CELL_VIDEO_OUT_RESOLUTION_576: 
			w=720;  h=576;  
			return 1;
		case CELL_VIDEO_OUT_RESOLUTION_720: 
			w=1280; h=720;  
			return 1;
		case CELL_VIDEO_OUT_RESOLUTION_1080: 
			w=1920; h=1080; 
			return 1;
		case CELL_VIDEO_OUT_RESOLUTION_1600x1080: 
			w=1600; h=1080; 
			return 1;
		case CELL_VIDEO_OUT_RESOLUTION_1440x1080: 
			w=1440; h=1080; 
			return 1;
		case CELL_VIDEO_OUT_RESOLUTION_1280x1080: 
			w=1280; h=1080; 
			return 1;
		case CELL_VIDEO_OUT_RESOLUTION_960x1080: 
			w=960;  h=1080; 
			return 1;
	};

	// error...
	return 0;
}

bool videoOutIsReady()
{
	CellVideoOutState videoState;

	cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);

	return(videoState.state == CELL_VIDEO_OUT_OUTPUT_STATE_ENABLED);
}



int main()
{
	renderWidth = 0;
	renderHeight = 0;
	deviceWidth = 0;
	deviceHeight = 0;
	
	frames = 0;
	while (!videoOutIsReady())
	{
		// ...
	}

	PSGLinitOptions options =
	{
		enable:					PSGL_INIT_MAX_SPUS | PSGL_INIT_INITIALIZE_SPUS,
		maxSPUs : 1,
		initializeSPUs : GL_FALSE,
		persistentMemorySize : 0,
		transientMemorySize : 0,
		errorConsole : 0,
		fifoSize : 0,
		hostMemorySize : 128 * 1024 * 1024,  // 128 mbs for host memory 
	};

#if CELL_SDK_VERSION < 0x340000
	options.enable |= PSGL_INIT_HOST_MEMORY_SIZE;
#endif

	// Initialize 6 SPUs but reserve 1 SPU as a raw SPU for PSGL
	sys_spu_initialize(6, 1);
	psglInit(&options);

	const unsigned int resolutions[] = {
		CELL_VIDEO_OUT_RESOLUTION_1080,
		CELL_VIDEO_OUT_RESOLUTION_720,
		CELL_VIDEO_OUT_RESOLUTION_480
	};

	const int numResolutions = sizeof(resolutions) / sizeof(resolutions[0]);

	int bestResolution = chooseBestResolution(resolutions, numResolutions);

	getResolutionWidthHeight(bestResolution, deviceWidth, deviceHeight);

	if (bestResolution)
	{
		PSGLdeviceParameters params;

		params.enable = PSGL_DEVICE_PARAMETERS_COLOR_FORMAT |
			PSGL_DEVICE_PARAMETERS_DEPTH_FORMAT |
			PSGL_DEVICE_PARAMETERS_MULTISAMPLING_MODE;
		params.colorFormat = GL_ARGB_SCE;
		params.depthFormat = GL_NONE;
		params.multisamplingMode = GL_MULTISAMPLING_NONE_SCE;
		params.enable |= PSGL_DEVICE_PARAMETERS_WIDTH_HEIGHT;
		params.width = deviceWidth;
		params.height = deviceHeight;

		device = psglCreateDeviceExtended(&params);
		context = psglCreateContext();

		psglMakeCurrent(context, device);
		psglResetCurrentContext();

		initGraphics();

		dbgFontInit();

		// Piece of code about Ps3MapiDyn
		int tmp_offset = 0;
		char screen_buffer_out[256];
		int result1, result2, ret;

		if (ps3mapidyn_init())
			sprintf(screen_buffer_out, "\n\nPS3 MAPI DYNAREC TEST\n\nSORRY... MAPI NOT PRESENT");
		else
		{
			char *buf = NULL;
			buf = (char*)malloc(100);
			ret = ps3mapidyn_write_bytecode(tmp_offset, fret5, 16);
			if (ret)
				NOP;  //Error

			int(*func)() = (int (*)())START_DYNAREC_BUFFER;
			result1 = func(); // result1 should be 5

			tmp_offset += 16 + DYNAREC_ADDRESS_SHIFT; // move to next position available

			ret = ps3mapidyn_write_bytecode(tmp_offset, fret9, 16);
			if (ret)
				NOP;  //Error

			func = (int (*)())((uint64_t)START_DYNAREC_BUFFER + tmp_offset);
			result2 = func(); // result2 should be 9

			// Prepare screen_buffer_out for DebugFont
			sprintf(screen_buffer_out, "\n\nPS3 MAPI DYNAREC TEST (PSGL VERSION)\n\n"
				"\nPS3 MAPI CORE VERSION: %d\n"
				"\nPS3 MAPI FW VERSION: %d\n"
				"\nPROCESS PID: %d"
				"\nDYNAREC BYTECODE BUFFER AT: 0x%llx "
				"\nLENGTH DYNAREC BYTECODE BUFFER: %d bytes"
				"\nRESULT FIRST FUNCTION CALL: %d (%s)"
				"\nRESULT SECOND FUNCYION CALL: %d (%s)",
				ps3mapi_get_core_version(), ps3mapi_get_fw_version(),
				sysProcessGetPid(), (uint64_t)START_DYNAREC_BUFFER, LEN_DYNAREC_BUFFER,
				result1, result1 == 5 ? "SUCCESS" : "FAIL", result2, result2 == 9 ? "SUCCESS" : "FAIL");
		}
		//////////////////////////////
		while (frames <= 5000)
		{
			frames++;
			cellDbgFontPrintf(0.100f, 0.200f, 0.8000f, 0xffffffff, screen_buffer_out);
			onRender();
			
			cellSysutilCheckCallback();
		}

		onShutdown();

	}
	else {
		// resolution error...
	}

	return 0;
}
