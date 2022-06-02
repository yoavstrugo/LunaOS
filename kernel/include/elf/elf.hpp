#pragma once

#include <stdint.h>
#include <tasking/tasking.hpp>
#include <fatfs/ff.h>

#define ELF_MAGIC 0x464C457FU // "\x7FELF" in little endian
#define ELF_NIDENT 16

typedef uint64_t Elf64_Addr;  // Unsigned address
typedef uint64_t Elf64_Off;   // Unsigned offset
typedef uint16_t Elf64_Half;  // Unsigned half int
typedef uint32_t Elf64_Word;  // Unsigned int
typedef int32_t Elf64_Sword;  // Signed int
typedef uint64_t Elf64_Xword; // Signed int
typedef int64_t Elf64_Sxword; // Signed int

enum Elf_Ident
{
    EI_MAG0 = 0,       // 0x7F
    EI_MAG1 = 1,       // 'E'
    EI_MAG2 = 2,       // 'L'
    EI_MAG3 = 3,       // 'F'
    EI_CLASS = 4,      // Architecture (32/64)
    EI_DATA = 5,       // Byte Order
    EI_VERSION = 6,    // ELF Version
    EI_OSABI = 7,      // OS Specific
    EI_ABIVERSION = 8, // OS Specific
    EI_PAD = 9         // Padding
};

#define ELFMAG0 0x7F // e_ident[EI_MAG0]
#define ELFMAG1 'E'  // e_ident[EI_MAG1]
#define ELFMAG2 'L'  // e_ident[EI_MAG2]
#define ELFMAG3 'F'  // e_ident[EI_MAG3]

#define ELFDATA2LSB (1) // Little Endian
#define ELFDATA2MSB (2) // Big Endian

#define ELFCLASS32 (1) // 32-bit Architecture
#define ELFCLASS64 (2) // 32-bit Architecture

#define ELFOSABI_SYSV 0         // System V ABI
#define ELFOSABI_HPUX 1         // HP-UX operating system
#define ELFOSABI_STANDALONE 255 // Standalone (embedded) application

enum Elf_Type
{
    ET_NONE = 0, // Unkown Type
    ET_REL = 1,  // Relocatable File
    ET_EXEC = 2  // Executable File
};

#define EM_386 (0x3)     // x86 Machine Type
#define EM_X86_64 (0x3E) // x86_64 Machine Type
#define EV_CURRENT (1)   // ELF Current Version

// File header
struct Elf64_Ehdr
{
    unsigned char e_ident[16]; /* ELF identification */
    Elf64_Half e_type;         /* Object file type */
    Elf64_Half e_machine;      /* Machine type */
    Elf64_Word e_version;      /* Object file version */
    Elf64_Addr e_entry;        /* Entry point address */
    Elf64_Off e_phoff;         /* Program header offset */
    Elf64_Off e_shoff;         /* Section header offset */
    Elf64_Word e_flags;        /* Processor-specific flags */
    Elf64_Half e_ehsize;       /* ELF header size */
    Elf64_Half e_phentsize;    /* Size of program header entry */
    Elf64_Half e_phnum;        /* Number of program header entries */
    Elf64_Half e_shentsize;    /* Size of section header entry */
    Elf64_Half e_shnum;        /* Number of section header entries */
    Elf64_Half e_shstrndx;     /* Section name string table index */
};

#define PT_NULL 0            // Unused entry
#define PT_LOAD 1            // Loadable segment
#define PT_DYNAMIC 2         // Dynamic linking tables
#define PT_INTERP 3          // Program interpreter path name
#define PT_NOTE 4            // Note sections
#define PT_SHLIB 5           // Reserved
#define PT_PHDR 6            // Program header table
#define PT_LOOS 0x60000000   // Environment-specific use low bytes
#define PT_HIOS 0x6FFFFFFF   // Environment-specific use high bytes
#define PT_LOPROC 0x70000000 // Processor-specific use low bytes
#define PT_HIPROC 0x7FFFFFFF // Processor-specific use high bytes

enum Elf_pflags
{
    PF_X = 0x1,              // Execute permission
    PF_W = 0x2,              // Write permission
    PF_R = 0x4,              // Read permission
    PF_MASKOS = 0x00FF0000,  // These flag bits are reserved for environment-specific use
    PF_MASKPROC = 0xFF000000 // These flag bits are reserved for processor-specific use
};

// Program section header
struct Elf64_Phdr
{
    Elf64_Word p_type;    /* Type of segment */
    Elf64_Word p_flags;   /* Segment attributes */
    Elf64_Off p_offset;   /* Offset in file */
    Elf64_Addr p_vaddr;   /* Virtual address in memory */
    Elf64_Addr p_paddr;   /* Reserved */
    Elf64_Xword p_filesz; /* Size of segment in file */
    Elf64_Xword p_memsz;  /* Size of segment in memory */
    Elf64_Xword p_align;  /* Alignment of segment */
}__attribute__((packed));

struct Elf64_Shdr
{
    Elf64_Word sh_name;       /* Section name */
    Elf64_Word sh_type;       /* Section type */
    Elf64_Xword sh_flags;     /* Section attributes */
    Elf64_Addr sh_addr;       /* Virtual address in memory */
    Elf64_Off sh_offset;      /* Offset in file */
    Elf64_Xword sh_size;      /* Size of section */
    Elf64_Word sh_link;       /* Link to other section */
    Elf64_Word sh_info;       /* Miscellaneous information */
    Elf64_Xword sh_addralign; /* Address alignment boundary */
    Elf64_Xword sh_entsize;   /* Size of entries, if section has table */
};

// Values for Proghdr type
#define ELF_PROG_LOAD 1
#define ELF_PROG_DYNAMIC 2
#define ELF_PROG_INTERP 3

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC 1
#define ELF_PROG_FLAG_WRITE 2
#define ELF_PROG_FLAG_READ 4

bool elfCheckFile(Elf64_Ehdr *header);
bool elfCheckSupported(Elf64_Ehdr *hdr);

unsigned long
elf64Hash(const unsigned char *name);

enum ELF_LOAD_STATUS
{
    SUCCESS = 0,
    FAILED_TO_OPEN = 1, // Failed to open file
    INVALID_HEADER = 2,
    FAILED_TO_READ = 3 ,
    UNSUPPORTED_ELF = 4
};

/**
 * @brief Loads an ELF file and create a process with thread which executes it
 * 
 * @param fileName The path to the ELF file
 * @return ELF_LOAD_STATUS The status of the loading
 */
ELF_LOAD_STATUS
    elfLoad(char *fileName);