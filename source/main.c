#include <stdio.h>
#include <stdlib.h>
#include <sys/process.h>
#include <string.h>
#include <unistd.h>

#define DYN_SIZE(x)  DYN256B(x) // DYN64K(DYN24B(DYN8B(x))) -> 16384 nop * 6 nop * 2 nop = 786432 Bytes (plus overhead)
#include "ps3mapidyn.h"

char fret5[32] = {0xfb, 0xe1, 0xff, 0xf8, 0xf8, 0x21, 0xff, 0xc1, 0x7c, 0x3f, 0x0b, 0x78, 0x39, 0x20, 0x00, 0x05, 
				  0x7d, 0x23, 0x4b, 0x78, 0x38, 0x3f, 0x00, 0x40, 0xeb, 0xe1, 0xff, 0xf8, 0x4e, 0x80, 0x00, 0x20};
			  
char fret9[32] = {0xfb, 0xe1, 0xff, 0xf8, 0xf8, 0x21, 0xff, 0xc1, 0x7c, 0x3f, 0x0b, 0x78, 0x39, 0x20, 0x00, 0x09, 
				  0x7d, 0x23, 0x4b, 0x78, 0x38, 0x3f, 0x00, 0x40, 0xeb, 0xe1, 0xff, 0xf8, 0x4e, 0x80, 0x00, 0x20};

int main(int argc,const char *argv[])
{
	
	int result;
	int rit=1;
	
	char *buf = NULL;
	buf = (char*)malloc(100);
	
	if (ps3mapidyn_init())
		exit(1);  // mapi not present
			
	result = ps3mapi_get_process_mem(sysProcessGetPid(), (uint64_t)START_DYNAREC_BUFFER, buf, 100);

	int tmp_offset = 0;
	result = ps3mapidyn_write_bytecode(tmp_offset, fret5, 32);
	
	int(*func)() = (int (*)())START_DYNAREC_BUFFER;
	rit=func();
	
	tmp_offset += 32 + DYNAREC_ADDRESS_SHIFT;
	
	
	result = ps3mapidyn_write_bytecode(tmp_offset, fret9, 32);
	
	func = (int (*)())((uint64_t)START_DYNAREC_BUFFER + tmp_offset);
	rit=func();
	
	result = ps3mapi_get_process_mem(sysProcessGetPid(), (uint64_t)START_DYNAREC_BUFFER, buf, 100);	
	
    return 0;
}
