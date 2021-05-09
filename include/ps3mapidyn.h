#ifndef __PS3MAPIDYN_H__
#define __PS3MAPIDYN_H__

#define MEMORY_PAGE_SIZE_4K 0x100
#define MEMORY_PAGE_SIZE_64K 0x200
#define MEMORY_PAGE_SIZE_1M 0x400

#ifndef __PS3MAPI_H__
#define process_id_t					uint32_t

#define SYSCALL8_OPCODE_PS3MAPI			0x7777
#define PS3MAPI_OPCODE_SET_PROC_MEM		0x0032
#define PS3MAPI_CORE_MINVERSION			0x0120
#define PS3MAPI_OPCODE_GET_CORE_VERSION	0x0011
#define PS3MAPI_OPCODE_GET_PROC_MEM		0x0031
#endif

#define NOP __asm__("nop")  // a nop is 4Byte

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

void *start_dyn_buff;
int len_dyn_buff;

#define START_DYNAREC_BUFFER start_dyn_buff
#define LEN_DYNAREC_BUFFER len_dyn_buff
#define DYNAREC_ADDRESS_SHIFT 12

void FAKEFUN(void) 
{
#ifdef DYN_SIZE
	DYN_SIZE(NOP);
#else
	DYN32B(NOP);
#endif	
	
}
#endif /* __PS3MAPIDYN_H__ */