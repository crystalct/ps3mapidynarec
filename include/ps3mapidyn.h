
#ifndef __PSL1GHT__
#include <sys/process.h>
#define lv2syscall2 system_call_2
#define sysProcessGetPid sys_process_getpid
#endif

#ifdef __cplusplus
extern "C" {
#endif

int ps3mapidyn_write_bytecode(int offset, char *buff, int len);
int ps3mapidyn_init(void);

#ifdef __cplusplus
}
#endif

#ifndef __PS3MAPIDYN_H__
#define __PS3MAPIDYN_H__

#define MEMORY_PAGE_SIZE_4K 0x100
#define MEMORY_PAGE_SIZE_64K 0x200
#define MEMORY_PAGE_SIZE_1M 0x400

#ifndef __PS3MAPI_H__
#define process_id_t					uint32_t

#define SYSCALL8_OPCODE_PS3MAPI				0x7777
#define PS3MAPI_OPCODE_SET_PROC_MEM			0x0032
#define PS3MAPI_CORE_MINVERSION				0x0120
#define PS3MAPI_OPCODE_GET_CORE_VERSION		0x0011
#define PS3MAPI_OPCODE_GET_PROC_MEM			0x0031
#define PS3MAPI_OPCODE_PROC_PAGE_ALLOCATE	0x0033
#define PS3MAPI_OPCODE_PROC_PAGE_FREE		0x0034
#endif

#define NOP __asm__("nop")  // a nop is 4 Byte

#define DYN8B(a)	a;a
#define DYN16B(a)	a;a;a;a
#define DYN24B(a)	a;a;a;a;a;a
#define DYN32B(a)	a;a;a;a;a;a;a;a
#define DYN40B(a)	a;a;a;a;a;a;a;a;a;a
#define DYN48B(a)	a;a;a;a;a;a;a;a;a;a;a;a
#define DYN56B(a)	a;a;a;a;a;a;a;a;a;a;a;a;a;a
#define DYN64B(a)	a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a
#define DYN128B(a) 	DYN32B(DYN16B(a)) // 32 nop
#define DYN256B(a) 	DYN64B(DYN16B(a)) // 64 nop
#define DYN512B(a) 	DYN64B(DYN32B(a)) // 128 nop
#define DYN1K(a) 	DYN128B(DYN32B(a)) // 256 nop
#define DYN2K(a) 	DYN256B(DYN32B(a)) // 512 nop
#define DYN4K(a) 	DYN512B(DYN32B(a)) // 1024 nop
#define DYN8K(a) 	DYN1K(DYN32B(a)) // 2048 nop
#define DYN16K(a) 	DYN2K(DYN32B(a)) // 4096 nop
#define DYN32K(a) 	DYN4K(DYN32B(a)) // 8192 nop
#define DYN64K(a) 	DYN8K(DYN32B(a)) // 16384 nop
#define DYN128K(a) 	DYN16K(DYN32B(a)) // 32768 nop
#define DYN256K(a) 	DYN32K(DYN32B(a)) // 65536 nop
#define DYN512K(a) 	DYN64K(DYN32B(a)) // 131072 nop
#define DYN1M(a) 	DYN128K(DYN32B(a)) // 262144 nop
#define DYN2M(a) 	DYN256K(DYN32B(a)) // 524288 nop
#define DYN4M(a) 	DYN512K(DYN32B(a)) // 1048576 nop

#define START_DYNAREC_BUFFER start_dyn_buff
#define LEN_DYNAREC_BUFFER len_dyn_buff
#ifdef __PSL1GHT__
#define DYNAREC_ADDRESS_SHIFT 12
#else
#define DYNAREC_ADDRESS_SHIFT 8
#endif

#ifdef __cplusplus
extern "C" {
#endif

int ps3mapi_process_page_allocate(process_id_t pid, uint64_t size, uint64_t page_size, uint64_t flags, uint64_t is_executable, uint64_t *page_table);
int ps3mapi_process_page_free(process_id_t pid, uint64_t flags, uint64_t *page_table);

void *start_dyn_buff;
int len_dyn_buff;

#ifndef __PS3MAPIDYN_CODE__
#define __PS3MAPIDYN_CODE__
#ifndef __PS3MAPI_H__

int ps3mapi_process_page_allocate(process_id_t pid, uint64_t size, uint64_t page_size, uint64_t flags, uint64_t is_executable, uint64_t *page_table)
{
#ifdef __PSL1GHT__
	lv2syscall8(8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PROC_PAGE_ALLOCATE, (uint64_t)pid, (uint64_t)size, (uint64_t)page_size, (uint64_t)flags, (uint64_t)is_executable, (uint64_t)page_table);
#else
	system_call_8((uint64_t)8,(uint64_t)SYSCALL8_OPCODE_PS3MAPI,(uint64_t)PS3MAPI_OPCODE_PROC_PAGE_ALLOCATE,(uint64_t)pid,(uint64_t)size, (uint64_t)page_size, (uint64_t)flags, (uint64_t)is_executable, (uint64_t)page_table);
#endif
	return_to_user_prog(int);						
}

int ps3mapi_process_page_free(process_id_t pid, uint64_t flags, uint64_t *page_table)
{
#ifdef __PSL1GHT__
	lv2syscall5(8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PROC_PAGE_FREE, (uint64_t)pid, (uint64_t)flags, (uint64_t)page_table);
#else
	system_call_5((uint64_t)8,(uint64_t)SYSCALL8_OPCODE_PS3MAPI,(uint64_t)PS3MAPI_OPCODE_PROC_PAGE_FREE,(uint64_t)pid,(uint64_t)flags, (uint64_t)page_table);
#endif
	return_to_user_prog(int);						
}

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
#ifdef __PSL1GHT__
	lv2syscall6(8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_PROC_MEM, (uint64_t)pid, (uint64_t)addr, (uint64_t)buf, (uint64_t)size);
#else
	system_call_6((uint64_t)8,(uint64_t)SYSCALL8_OPCODE_PS3MAPI,(uint64_t)PS3MAPI_OPCODE_SET_PROC_MEM,(uint64_t)pid,(uint64_t)addr,(uint64_t)(uint32_t)buf,(uint64_t)size);
#endif
	return_to_user_prog(int);
}

int ps3mapi_get_process_mem(process_id_t pid, uint64_t addr, char *buf, int size)
{
#ifdef __PSL1GHT__
	lv2syscall6(8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_MEM, (uint64_t)pid, (uint64_t)addr, (uint64_t)buf, (uint64_t)size);
#else
	system_call_6((uint64_t)8,(uint64_t)SYSCALL8_OPCODE_PS3MAPI,(uint64_t)PS3MAPI_OPCODE_GET_PROC_MEM,(uint64_t)pid,(uint64_t)addr,(uint64_t)(uint32_t)buf,(uint64_t)size);
#endif
	return_to_user_prog(int);						
}
#endif // __PS3MAPI_H__

int ps3mapidyn_write_bytecode(int offset, char *buff, int len)
{
	if (offset + len > LEN_DYNAREC_BUFFER || offset < 0 || !buff || offset%4)
		return 1;

	char *tmp = (char*)malloc(len+DYNAREC_ADDRESS_SHIFT);
	if(!tmp)
		return 1;

#ifdef __PSL1GHT__
	*(uint64_t *)tmp = (uint64_t)START_DYNAREC_BUFFER + offset + DYNAREC_ADDRESS_SHIFT;
	*(uint32_t *)(tmp+sizeof(uint64_t)) = 0;
#else
	*(uint32_t *)tmp = (uint32_t)START_DYNAREC_BUFFER + offset + DYNAREC_ADDRESS_SHIFT;
	*(uint32_t *)(tmp+sizeof(uint32_t)) = 0;
#endif
	memcpy((void*)(tmp+DYNAREC_ADDRESS_SHIFT), buff, len);
#ifdef __PSL1GHT__
	int result = ps3mapi_set_process_mem(sysProcessGetPid(), (uint64_t)START_DYNAREC_BUFFER + offset, tmp, len);
#else
	int result = ps3mapi_set_process_mem(sysProcessGetPid(), (uint32_t)START_DYNAREC_BUFFER + offset, tmp, len);
#endif
	free(tmp);

	if (result)
		return 1;

	return 0;
}

void FAKEFUN(void) 
{
#ifdef DYN_SIZE
	DYN_SIZE(NOP);
#else
	DYN32B(NOP);
#endif	
}

int ps3mapidyn_init()
{
	if (!has_ps3mapi()) 
		return 1; 	// Error, ps3mapi not present
	
#ifdef __PSL1GHT__
	START_DYNAREC_BUFFER = (void*)*(uint64_t*)FAKEFUN;
#else
	START_DYNAREC_BUFFER = (void*)*(uint32_t*)FAKEFUN;
#endif
	
	uint32_t *toffset = (uint32_t*)START_DYNAREC_BUFFER;
	int x = 0;

	while (*toffset != 0x4e800020)  //Search for blr
	{
		toffset++;
		x++;
	}

	LEN_DYNAREC_BUFFER = x*4 + 4;  //Dynarec buffer length;
	return 0;
}

#endif // __PS3MAPIDYN_CODE__

#ifdef __cplusplus
}
#endif


#endif /* __PS3MAPIDYN_H__ */
