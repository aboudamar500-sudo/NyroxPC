#ifndef NYROX_ELF64_H
#define NYROX_ELF64_H

typedef unsigned char Elf64_U8;
typedef unsigned short Elf64_U16;
typedef unsigned int Elf64_U32;
typedef unsigned long long Elf64_U64;

typedef struct {
    Elf64_U8 ident[16];
    Elf64_U16 type;
    Elf64_U16 machine;
    Elf64_U32 version;
    Elf64_U64 entry;
    Elf64_U64 program_header_offset;
    Elf64_U64 section_header_offset;
    Elf64_U32 flags;
    Elf64_U16 header_size;
    Elf64_U16 program_header_size;
    Elf64_U16 program_header_count;
    Elf64_U16 section_header_size;
    Elf64_U16 section_header_count;
    Elf64_U16 section_name_index;
} Elf64_Ehdr;

typedef struct {
    Elf64_U32 type;
    Elf64_U32 flags;
    Elf64_U64 offset;
    Elf64_U64 virtual_address;
    Elf64_U64 physical_address;
    Elf64_U64 file_size;
    Elf64_U64 memory_size;
    Elf64_U64 alignment;
} Elf64_Phdr;

#define ELF64_CLASS_64 2
#define ELF64_LITTLE_ENDIAN 1
#define ELF64_VERSION_CURRENT 1
#define ELF64_TYPE_EXEC 2

#define ELF64_PT_LOAD 1
#define ELF64_PF_EXECUTE 1
#define ELF64_MAX_PROGRAM_HEADERS 32
#define ELF64_MACHINE_X86_64 62

_Static_assert(sizeof(Elf64_Ehdr) == 64, "ELF64 header size error");
_Static_assert(sizeof(Elf64_Phdr) == 56, "ELF64 program header size error");


#endif
