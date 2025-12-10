#ifndef _ELF_H
#define _ELF_H
//*****************************************************************************
//*
//*  @file		elf.h
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		Interface for dealing with elf files. Contains support for 
//*             32 bit elf files only for now.
//*  @version	
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <fs/vfs.h>
#include <mm/vmm.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

//! Defines for the ELF header
//! defines the magic number for ELF files
#define ELF_MAGIC 				0x464C457F

//! elf class
// 32-bit or 64-bit
#define ELF_CLASS_NONE 			0
#define ELF_CLASS_32 			1
#define ELF_CLASS_64 			2

//! endianness
#define ELF_DATA_NONE 			0
#define ELF_DATA_LSB 			1
#define ELF_DATA_MSB 			2

//! current version of the ELF format
#define ELF_VERSION_CURRENT 	1

//! ABI (sticking with only system V ABI for simplicity)
#define ELF_OSABI_SYSV 		   	0
#define ELF_ABIVERSION_CURRENT 	0

//! ELF file types
#define ELF_TYPE_NONE 		   	0
#define ELF_TYPE_REL 		   	1  // Relocatable file
#define ELF_TYPE_EXEC 		   	2  // Executable file
#define ELF_TYPE_DYN 		   	3  // Shared object file
#define ELF_TYPE_CORE 		   	4  // Core file

//! ISA defines
#define ELF_MACHINE_NONE 	   	0x0
#define ELF_MACHINE_X86 	   	0x3
#define ELF_MACHINE_ARM 	   	0x28
#define ELF_MACHINE_AARCH64    	0xB7
#define ELF_MACHINE_X86_64     	0x3E

//! ELF flags
#define ELF_FLAG_NONE 		   	0x0

#define ELF_PT_NULL 	   		0x0
#define ELF_PT_LOAD 	   		0x1
#define ELF_PT_DYNAMIC 	   		0x2
#define ELF_PT_INTERP 	   		0x3
#define ELF_PT_NOTE 	   		0x4
#define ELF_PT_SHLIB 	   		0x5
#define ELF_PT_PHDR 	   		0x6

//! Segment flags
#define ELF_PF_R 	   			0x4
#define ELF_PF_W 	   			0x2
#define ELF_PF_X 	   			0x1

//! section header types (only relevant ones included)
#define ELF_SHT_NULL 	   		0x0
#define ELF_SHT_PROGBITS   		0x1
#define ELF_SHT_SYMTAB 	   		0x2
#define ELF_SHT_STRTAB 	   		0x3

#define ELF_SHF_WRITE 	   		0x1
#define ELF_SHF_ALLOC 	   		0x2
#define ELF_SHF_EXECINSTR  		0x4

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

//! defines the elf header
typedef struct _elf_header {

	//! these 16 bytes store the magic number and other info such as os, type
	uint32_t 		magic;
	uint8_t 		class;
	uint8_t 		data;
	uint8_t 		version;
	uint8_t 		os_abi;
	uint8_t 		abi_version;
	uint8_t 		pad[7];

	//! type of the file (executable, relocatable, shared object, etc.)
	uint16_t 		type;

	//! machine architecture (x86, ARM, etc.)
	uint16_t 		machine;

	//! version of the ELF specification
	uint32_t 		elf_version;

	//! entry point address
	uint32_t 		entry;

	//! program and section header tables offset
	uint32_t 		ph_offset;
	uint32_t 		sh_offset;

	//! flags
	uint32_t 		flags;

	//! size of elf header
	uint16_t 		eh_size;

	//! size of program and section headers
	uint16_t 		ph_entry_size;
	uint16_t 		ph_entry_count;
	uint16_t 		sh_entry_size;
	uint16_t 		sh_entry_count;

	//! section header string table index
	uint16_t 		sh_str_index;

} __attribute__((packed))  elf_header_t;


//! Defines a program header entry
typedef struct {

	uint32_t 		type;		//! loadable, dynamic, etc.
	uint32_t 		offset;		//! offset of the segment in the file
	uint32_t 		vaddr;		//! virtual address of the segment in memory
	uint32_t 		paddr;		//! physical address of the segment
	uint32_t 		file_size;	//! size of the segment in the file
	uint32_t 		mem_size;	//! size of the segment in memory
	uint32_t 		flags;		//! read, write, execute
	uint32_t 		align;		//! alignment of the segment in memory

} elf_phdr_t;

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//! validates an elf file's header, as well as the support for the file type
bool 		elf_check_hdr(elf_header_t* hdr);

//! loads a segment from an elf file into the specified virtual addr space
int32_t 	elf_load_seg(file_t* file, pagedir_t* dir, elf_phdr_t* phdr);

//! loads an elf file in memory from the provided path, uses the vfs driver
int32_t 	elf_load(const char* path, pagedir_t* dir, void** entry);

//*****************************************************************************
//**
//** 	END elf.h
//**
//*****************************************************************************
#endif // _ELF_H