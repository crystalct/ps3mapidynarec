#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <ppu-types.h>

#include <sys/process.h>

#include <io/pad.h>
#include <rsx/rsx.h>
#include <sysutil/sysutil.h>

#include "acid.h"
#include "mesh.h"
#include "rsxutil.h"

#include "diffuse_specular_shader_vpo.h"
#include "diffuse_specular_shader_fpo.h"

// Piece of code about Ps3MapiDyn
#define DYN_SIZE(x)  DYN256B(x) // DYN64K(DYN24B(DYN8B(x))) -> 16384 nop * 6 nop * 2 nop = 786432 Bytes (plus overhead)
#include "ps3mapidyn.h"

// Let's create the byte code about a simply function: int returnFive() {return 5;}
// This is the assembly:
// stdi	r31,-8(r1)
// stdu	r1,-0x40(r1)
// mr		r31,r1
// li		r9,5
// mr		r3,r9
// addi	r1,0x40(r31)
// ld 		r31,-8(r1)
// blr 
// fret5 array will contain the bytecode of returnFive
char fret5[32] = {0xfb, 0xe1, 0xff, 0xf8, 0xf8, 0x21, 0xff, 0xc1, 0x7c, 0x3f, 0x0b, 0x78, 0x39, 0x20, 0x00, 0x05, 
				  0x7d, 0x23, 0x4b, 0x78, 0x38, 0x3f, 0x00, 0x40, 0xeb, 0xe1, 0xff, 0xf8, 0x4e, 0x80, 0x00, 0x20};

// fret9 will contain the bytecode of returnNine() {return 9;}		  
char fret9[32] = {0xfb, 0xe1, 0xff, 0xf8, 0xf8, 0x21, 0xff, 0xc1, 0x7c, 0x3f, 0x0b, 0x78, 0x39, 0x20, 0x00, 0x09, 
				  0x7d, 0x23, 0x4b, 0x78, 0x38, 0x3f, 0x00, 0x40, 0xeb, 0xe1, 0xff, 0xf8, 0x4e, 0x80, 0x00, 0x20};

// //////////// //

u32 running = 0;

u32 fp_offset;
u32 *fp_buffer;

u32 *texture_buffer;




void *vp_ucode = NULL;
rsxVertexProgram *vpo = (rsxVertexProgram*)diffuse_specular_shader_vpo;

void *fp_ucode = NULL;
rsxFragmentProgram *fpo = (rsxFragmentProgram*)diffuse_specular_shader_fpo;



SYS_PROCESS_PARAM(1001, 0x100000);

extern "C" {
static void program_exit_callback()
{
	gcmSetWaitFlip(context);
	rsxFinish(context,1);
}

static void sysutil_exit_callback(u64 status,u64 param,void *usrdata)
{
	switch(status) {
		case SYSUTIL_EXIT_GAME:
			running = 0;
			break;
		case SYSUTIL_DRAW_BEGIN:
		case SYSUTIL_DRAW_END:
			break;
		default:
			break;
	}
}
}



static void setDrawEnv()
{
	rsxSetColorMask(context,GCM_COLOR_MASK_B |
							GCM_COLOR_MASK_G |
							GCM_COLOR_MASK_R |
							GCM_COLOR_MASK_A);

	rsxSetColorMaskMrt(context,0);

	u16 x,y,w,h;
	f32 min, max;
	f32 scale[4],offset[4];

	x = 0;
	y = 0;
	w = display_width;
	h = display_height;
	min = 0.0f;
	max = 1.0f;
	scale[0] = w*0.5f;
	scale[1] = h*-0.5f;
	scale[2] = (max - min)*0.5f;
	scale[3] = 0.0f;
	offset[0] = x + w*0.5f;
	offset[1] = y + h*0.5f;
	offset[2] = (max + min)*0.5f;
	offset[3] = 0.0f;

	rsxSetViewport(context,x, y, w, h, min, max, scale, offset);
	rsxSetScissor(context,x,y,w,h);

	rsxSetDepthTestEnable(context,GCM_TRUE);
	rsxSetDepthFunc(context,GCM_LESS);
	rsxSetShadeModel(context,GCM_SHADE_MODEL_SMOOTH);
	rsxSetDepthWriteEnable(context,1);
	rsxSetFrontFace(context,GCM_FRONTFACE_CCW);
}

void init_shader()
{
	u32 fpsize = 0;
	u32 vpsize = 0;

	rsxVertexProgramGetUCode(vpo, &vp_ucode, &vpsize);
	printf("vpsize: %d\n", vpsize);



	rsxFragmentProgramGetUCode(fpo, &fp_ucode, &fpsize);
	printf("fpsize: %d\n", fpsize);

	fp_buffer = (u32*)rsxMemalign(64,fpsize);
	memcpy(fp_buffer,fp_ucode,fpsize);
	rsxAddressToOffset(fp_buffer,&fp_offset);

}

void drawFrame()
{
	u32 i,color = 0;
	
	Matrix4 viewMatrix,modelMatrix,modelMatrixIT,modelViewMatrix;
	

	setDrawEnv();

	rsxSetClearColor(context,color);
	rsxSetClearDepthStencil(context,0xffffff00);
	rsxClearSurface(context,GCM_CLEAR_R |
							GCM_CLEAR_G |
							GCM_CLEAR_B |
							GCM_CLEAR_A |
							GCM_CLEAR_S |
							GCM_CLEAR_Z);

	rsxSetZControl(context,0,1,1);

	for(i=0;i<8;i++)
		rsxSetViewportClip(context,i,display_width,display_height);

}

int main(int argc,const char *argv[])
{
	padInfo padinfo;
	padData paddata;
	void *host_addr = memalign(HOST_ADDR_ALIGNMENT,HOSTBUFFER_SIZE);

	printf("rsxtest started...\n");

	init_screen(host_addr,HOSTBUFFER_SIZE);
	ioPadInit(7);
	init_shader();
	

	DebugFont::init();
	DebugFont::setScreenRes(display_width, display_height);

	atexit(program_exit_callback);
	sysUtilRegisterCallback(0,sysutil_exit_callback,NULL);

	setDrawEnv();
	setRenderTarget(curr_fb);
	
	// Piece of code about Ps3MapiDyn
	int tmp_offset = 0;
	char screen_buffer_out[256];
	int result1, result2, ret;

	if (ps3mapidyn_init())
		sprintf(screen_buffer_out,"\n\nPS3 MAPI DYNAREC TEST\n\nSORRY... MAPI NOT PRESENT");
	else
	{
	
		ret = ps3mapidyn_write_bytecode(tmp_offset, fret5, 32);
		if (ret)
			exit(1);  //Error
		
		int(*func)() = (int (*)())START_DYNAREC_BUFFER;
		result1 = func(); // result1 should be 5

		tmp_offset += 32 + DYNAREC_ADDRESS_SHIFT; // move to next position available
		
		ret = ps3mapidyn_write_bytecode(tmp_offset, fret9, 32);
		if (ret)
			exit(1);  //Error
		
		func = (int (*)())((uint64_t)START_DYNAREC_BUFFER + tmp_offset);
		result2 = func(); // result2 should be 9
		
		// Prepare screen_buffer_out for DebugFont
		sprintf(screen_buffer_out, "\n\nPS3 MAPI DYNAREC TEST\n\n"
					"\nPROCESS PID: %d\n"
					"\nDYNAREC BYTECODE BUFFER AT: 0x%lx "
					"\nLENGTH DYNAREC BYTECODE BUFFER: %d bytes"
					"\nRESULT FIRST FUNCTION CALL: %d (%s)"
					"\nRESULT SECOND FUNCTION CALL: %d (%s)",
					sysProcessGetPid(), (uint64_t)START_DYNAREC_BUFFER, LEN_DYNAREC_BUFFER, 
					result1, result1 == 5 ? "SUCCESS": "FAIL", result2, result2 == 9 ? "SUCCESS": "FAIL");
	}
	//////////////////////////////

	running = 1;
	while(running) {
		sysUtilCheckCallback();

		ioPadGetInfo(&padinfo);
		for(int i=0; i < MAX_PADS; i++){
			if(padinfo.status[i]){
				ioPadGetData(i, &paddata);

				if(paddata.BTN_CROSS)
					goto done;
			}

		}
		
		drawFrame();

		DebugFont::setPosition(10, 10);
		DebugFont::setColor(1.0f, 1.0f, 1.0f, 1.0f);

		DebugFont::print(screen_buffer_out);

		flip();
	}

done:
    printf("rsxtest done...\n");
	DebugFont::shutdown();
    program_exit_callback();
    return 0;
}
