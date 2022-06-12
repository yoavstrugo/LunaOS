#include <elf/elf.hpp>
#include <logger/logger.hpp>
#include <stddef.h>
#include <strings.hpp>
#include <memory/memory.hpp>
#include <memory/heap.hpp>

bool elfCheckFile(Elf64_Ehdr *header)
{
    if (!header)
        return false;
    if (header->e_ident[EI_MAG0] != ELFMAG0)
    {
        logWarnn("ELF Header EI_MAG0 incorrect");
    }
    if (header->e_ident[EI_MAG1] != ELFMAG1)
    {
        logWarnn("ELF Header EI_MAG1 incorrect");
        return false;
    }
    if (header->e_ident[EI_MAG2] != ELFMAG2)
    {
        logWarnn("ELF Header EI_MAG2 incorrect");
        return false;
    }
    if (header->e_ident[EI_MAG3] != ELFMAG3)
    {
        logWarnn("ELF Header EI_MAG3 incorrect");
        return false;
    }
    logDebugn("ELF Header is correct");
    return true;
}

bool elfCheckSupported(Elf64_Ehdr *hdr)
{
    if (hdr->e_ident[EI_CLASS] != ELFCLASS64)
    {
        logWarnn("Unsupported ELF File Class.\n");
        return false;
    }
    if (hdr->e_ident[EI_DATA] != ELFDATA2LSB)
    {
        logWarnn("Unsupported ELF File byte order.\n");
        return false;
    }
    if (hdr->e_machine != EM_X86_64)
    {
        logWarnn("Unsupported ELF File target.\n");
        return false;
    }
    if (hdr->e_ident[EI_VERSION] != EV_CURRENT)
    {
        logWarnn("Unsupported ELF File version.\n");
        return false;
    }
    if (hdr->e_type != ET_REL && hdr->e_type != ET_EXEC)
    {
        logWarnn("Unsupported ELF File type.\n");
        return false;
    }
    return true;
}

unsigned long
elf64Hash(const unsigned char *name)
{
    unsigned long h = 0, g;
    while (*name)
    {
        h = (h << 4) + *name++;
        if (g = h & 0xf0000000)
            h ^= g >> 24;
        h &= 0x0fffffff;
    }
    return h;
}

ELF_LOAD_STATUS elfLoad(char *fileName, const char * name, k_process *proc)
{
    // First open the file
    FIL elfFile;
    FRESULT res;

    if ((res = f_open(&elfFile, fileName, FA_READ)) != FR_OK)
    {
        return FAILED_TO_OPEN;
    }

    unsigned int readBytes;
    uint64_t fileSize = f_size(&elfFile);
    char fileBuffer[fileSize];
    res = f_read(&elfFile, (void *)fileBuffer, fileSize, &readBytes);

    // Validate the file
    if (res != FR_OK)
    {
        f_close(&elfFile);
        return FAILED_TO_READ;
    }
    if (readBytes != fileSize)
    {
        f_close(&elfFile);
        return FAILED_TO_READ;
    }

    Elf64_Ehdr elfHeader;
    memcpy(&elfHeader, fileBuffer, sizeof(Elf64_Ehdr));

    if (!elfCheckFile(&elfHeader))
    {
        f_close(&elfFile);
        return INVALID_HEADER;
    }
    if (!elfCheckSupported(&elfHeader))
    {
        f_close(&elfFile);
        return UNSUPPORTED_ELF;
    }

    // File is valid, start loading it
    uint64_t phdrCout = elfHeader.e_phnum;
    uint64_t phdrSize = elfHeader.e_phentsize;
    uint64_t phdrOff = elfHeader.e_phoff;

    // Read all the program headers
    Elf64_Phdr phdrs[phdrCout];
    memcpy(phdrs, &fileBuffer[phdrOff], sizeof(phdrs));
    // if ((res = f_read(&elfFile, phdrs, phdrCout * phdrSize, &readBytes)) != SUCCESS)
    // {
    //     f_close(&elfFile);
    //     return FAILED_TO_READ;
    // }
    // if (readBytes != phdrCout * phdrSize)
    // {
    //     f_close(&elfFile);
    //     return FAILED_TO_READ;
    // }

    k_process *process = taskingCreateProcess();
    // Switch to the process' space to map everything
    physical_address_t prevSpace = pagingGetCurrentSpace();
    pagingSwitchSpace(process->addressSpace);
    elfFile.fptr = 0;
    for (int phdrI = 0; phdrI < phdrCout; phdrI++)
    {
        // Load the segments

        if (phdrs[phdrI].p_type == PT_LOAD)
        {
            // elfFile.fptr = phdrs[phdrI].p_offset;
            process->processAllocator->allocateRange(phdrs[phdrI].p_vaddr, phdrs[phdrI].p_memsz);

            memcpy((void *)phdrs[phdrI].p_vaddr, &fileBuffer[phdrs[phdrI].p_offset], phdrs[phdrI].p_filesz);
        }
        else if (phdrs[phdrI].p_type == PT_TLS)
        {
            size_t masterTLSAlignment = phdrs[phdrI].p_align;
            size_t masterTLSCopySize = phdrs[phdrI].p_filesz;

            process->masterTLS.alignment = phdrs[phdrI].p_align;;
            process->masterTLS.actualSize = phdrs[phdrI].p_filesz;
            process->masterTLS.totalSize = phdrs[phdrI].p_memsz;

            process->masterTLS.location = (virtual_address_t)heapAllocate(process->masterTLS.actualSize);          

            memset((char *)process->masterTLS.location, 0, process->masterTLS.actualSize);
            // Copy the TLS
            memcpy((void *)process->masterTLS.location, &fileBuffer[phdrs[phdrI].p_offset], process->masterTLS.actualSize);
        }
    }
    pagingSwitchSpace(prevSpace);

    f_close(&elfFile);

    // Create the process' main thread
    k_thread *thread = taskingCreateThread(elfHeader.e_entry, process, USER);

    return SUCCESS;
}