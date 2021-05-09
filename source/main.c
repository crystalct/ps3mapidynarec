#include <stdio.h>
#include <stdlib.h>
#include <sys/process.h>
#include <string.h>

#define DYN_SIZE(x)  DYN256B(x) // DYN64K(DYN24B(DYN8B(x))) -> 16384 nop * 6 nop * 2 nop = 786432 Bytes (plus overhead)
#include "ps3mapidyn.h"

#define STR_(X) #X

// this makes sure the argument is expanded before converting to string
#define STR(X) STR_(X)

//SYS_PROCESS_PARAM(1001, 0x100000);

FILE *fdebug;

char fret5[32] = {0xfb, 0xe1, 0xff, 0xf8, 0xf8, 0x21, 0xff, 0xc1, 0x7c, 0x3f, 0x0b, 0x78, 0x39, 0x20, 0x00, 0x05, 
				  0x7d, 0x23, 0x4b, 0x78, 0x38, 0x3f, 0x00, 0x40, 0xeb, 0xe1, 0xff, 0xf8, 0x4e, 0x80, 0x00, 0x20};
			  
char fret9[32] = {0xfb, 0xe1, 0xff, 0xf8, 0xf8, 0x21, 0xff, 0xc1, 0x7c, 0x3f, 0x0b, 0x78, 0x39, 0x20, 0x00, 0x09, 
				  0x7d, 0x23, 0x4b, 0x78, 0x38, 0x3f, 0x00, 0x40, 0xeb, 0xe1, 0xff, 0xf8, 0x4e, 0x80, 0x00, 0x20};

#ifndef __PS3MAPI_H__
int ps3mapi_get_core_version(void)
{
	lv2syscall2(8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_CORE_VERSION);
	return_to_user_prog(int);						
}

int has_ps3mapi(void)
{
	return (ps3mapi_get_core_version() >= PS3MAPI_CORE_MINVERSION);
}


int ps3mapi_set_process_mem(process_id_t pid, uint64_t addr, char *buf, int size )
{
	lv2syscall6(8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_PROC_MEM, (uint64_t)pid, (uint64_t)addr, (uint64_t)buf, (uint64_t)size);
	return_to_user_prog(int);
}

int ps3mapi_get_process_mem(process_id_t pid, uint64_t addr, char *buf, int size)
{
	lv2syscall6(8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_MEM, (uint64_t)pid, (uint64_t)addr, (uint64_t)buf, (uint64_t)size);
	return_to_user_prog(int);						
}
#endif 

int write_bytecode(int offset, char *buff, int len)
{
	fprintf(fdebug, "BYTECODE AT: %p - shift: 0x%lx\n", START_DYNAREC_BUFFER, (uint64_t)START_DYNAREC_BUFFER + offset + DYNAREC_ADDRESS_SHIFT);
	fflush(fdebug);
	if (offset + len > LEN_DYNAREC_BUFFER || !buff || offset%4)
		return 1;
	fprintf(fdebug, "write_bytecode: IF OK\n");
	fflush(fdebug);
	char *tmp = (char*)malloc(len+DYNAREC_ADDRESS_SHIFT);
	if(!tmp)
		return 1;
	
	*(uint64_t *)tmp= (uint64_t)START_DYNAREC_BUFFER + offset + DYNAREC_ADDRESS_SHIFT;
	*(uint32_t *)(tmp+sizeof(uint64_t))=0;
	memcpy((void*)(tmp+DYNAREC_ADDRESS_SHIFT), buff, len);
	int result = ps3mapi_set_process_mem(sysProcessGetPid(), (uint64_t)START_DYNAREC_BUFFER + offset, tmp, len);
	free(tmp);
	fprintf(fdebug, "Result write_bytecode: %d\n", result);
	fflush(fdebug);
	if (result)
		return 1;
	return 0;
}


int main(int argc,const char *argv[])
{
	
	void *app = &FAKEFUN;
	START_DYNAREC_BUFFER = (void*)*(uint64_t*)app;
	int rit=1;
	printf("APP: %p\n", app);
	
	//printf("AA" STR(DYN4K(NOP)) "\n");
	printf("ADD: 0x%lx\n", *(uint64_t*)app);

	uint32_t *toffset = (uint32_t*)START_DYNAREC_BUFFER;
	int x = 0;
	while (*toffset != 0x4e800020)  //Search for blr
	{
		toffset++;
		x++;
	}
	LEN_DYNAREC_BUFFER = x*4 + 4;  //lunghezza buffer;
	printf("Lunghezza buffer: %d\n", LEN_DYNAREC_BUFFER);
	
	//sleep(5);
	fdebug = fopen("/dev_hdd0/tmp/mapi.log", "w");
	fprintf(fdebug, "Mapi start OK\n");
	fflush(fdebug);
	fprintf(fdebug, "MAPI: %d\n", has_ps3mapi());
	fflush(fdebug);
	
	fprintf(fdebug,"Process pid: 0x%x\n", sysProcessGetPid());
	fflush(fdebug);
	
	fprintf(fdebug, "FAKEFUN pointer: %p - BYTECODE AT: 0x%lx\n", app, *(uint64_t*)app);
	fflush(fdebug);
	
    //int result = ps3mapi_process_page_allocate(sysProcessGetPid(), size, page_size, 0x2F, 0x1, &page_addr);
    //fprintf(fdebug, "ps3mapi_process_page_allocate result: 0x%x\n", result);
	char *buf = NULL;
	buf = (char*)malloc(100);
	int result = ps3mapi_get_process_mem(sysProcessGetPid(), (uint64_t)START_DYNAREC_BUFFER, buf, 100);
	fprintf(fdebug, "ps3mapi_get_process_mem result: 0x%x\n", result);
	for(int i = 0; i < 100; i++)
		fprintf(fdebug, "[0x%x]", buf[i]);
	
	fprintf(fdebug, "\n");
	fflush(fdebug);
	//((uint8_t*)buf)[19] = 0xCA;
	//((uint8_t*)buf)[18] = 0xF1;
	// ((uint8_t*)buf)[22] = 0xCA;
	// ((uint8_t*)buf)[23] = 00;
	
	
	// *(uint64_t *)buf= *(uint64_t*)app + 12;
	// *(uint32_t *)(buf+8)=0;
	// *(uint64_t *)(buf+12)=0xfbe1fff8f821ffc1;
	// *(uint64_t *)(buf+20)=0x7c3f0b7839200005;
	// *(uint64_t *)(buf+28)=0x7d234b78383f0040;
	// *(uint64_t *)(buf+36)=0xebe1fff84e800020;
		
	// result = ps3mapi_set_process_mem(sysProcessGetPid(), *(uint64_t*)app, buf, 100);
	// fprintf(fdebug, "ps3mapi_set_process_mem result: 0x%x\n", result);
	// fflush(fdebug);
	int tmp_offset = 0;
	result = write_bytecode(tmp_offset, fret5, 32);
	fprintf(fdebug, "Return first write_bytecode: %d\n", result);
	fflush(fdebug);
	
	int(*func)()=(void *)START_DYNAREC_BUFFER;
	rit=func();
	fprintf(fdebug, "\nFirst rit: 0x%x\n", rit);
	fflush(fdebug);
	
	tmp_offset += 32 + DYNAREC_ADDRESS_SHIFT;
	
	
	result = write_bytecode(tmp_offset, fret9, 32);
	fprintf(fdebug, "Return second write_bytecode: %d\n", result);
	fflush(fdebug);
	
	func = (void *)(START_DYNAREC_BUFFER + tmp_offset);
	rit=func();
	fprintf(fdebug, "\nSecond rit: 0x%x\n", rit);
	fflush(fdebug);
	
	result = ps3mapi_get_process_mem(sysProcessGetPid(), (uint64_t)START_DYNAREC_BUFFER, buf, 100);
	fprintf(fdebug, "ps3mapi_get_process_mem result: 0x%x\n", result);
	fflush(fdebug);
	for(int i = 0; i < 100; i++)
		fprintf(fdebug, "[0x%x]", buf[i]);
	
	fprintf(fdebug, "\n");
	
	
	
	fprintf(fdebug, "\n0x%x\n", rit);
	fflush(fdebug);
	fclose(fdebug);
	
	
    return 0;
}
