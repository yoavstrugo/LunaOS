#include <kernel.hpp>

#include <logger/logger.hpp>
#include <logger/logger_macros.hpp>

#include <stivale2/stivale2.h>
#include <stivale2/stivale2_tools.hpp>

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include <strings.hpp>

#include <interrupts/interrupts.hpp>
#include <interrupts/pic.hpp>
#include <interrupts/lapic.hpp>
#include <interrupts/ioapic.hpp>

#include <gdt/gdt.hpp>
#include <memory/memory.hpp>
#include <memory/buddy_allocator.hpp>
#include <memory/heap.hpp>

#include <system/acpi/rsdp.hpp>
#include <system/acpi/acpi.hpp>
#include <system/acpi/madt.hpp>

#include <tasking/tasking.hpp>
#include <tasking/scheduler.hpp>

#include <syscalls/syscalls_tasking.hpp>

#include <fatfs/ff.h>

#include <storage/ahci/ahci.hpp>
#include <system/pci/pci.hpp>
#include <system/cmos.hpp>

#include <filesystem.hpp>
#include <elf/elf.hpp>

// We need to tell the stivale bootloader where we want our stack to be.
// We are going to allocate our stack as an array in .bss.
static uint8_t stack[0x100000];

// stivale2 uses a linked list of tags for both communicating TO the
// bootloader, or receiving info FROM it. More information about these tags
// is found in the stivale2 specification.

// We are now going to define a framebuffer header tag.
// This tag tells the bootloader that we want a graphical framebuffer instead
// of a CGA-compatible text mode. Omitting this tag will make the bootloader
// default to text mode, if available.
static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    // Same as above.
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        // Instead of 0, we now point to the previous header tag. The order in
        // which header tags are linked does not matter.
        .next = 0},
    // We set all the framebuffer specifics to 0 as we want the bootloader
    // to pick the best it can.
    .framebuffer_width = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp = 0,
    .unused = 0};

static stivale2_header_tag_smp smp_hdr_tag{
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_SMP_ID,
        .next = (uint64_t)&framebuffer_hdr_tag},
    .flags = 1UL};


// The stivale2 specification says we need to define a "header structure".
// This structure needs to reside in the .stivale2hdr ELF section in order
// for the bootloader to find it. We use this __attribute__ directive to
// tell the compiler to put the following structure in said section.
__attribute__((section(".stivale2hdr"), used)) static struct stivale2_header stivale_hdr = {
    // The entry_point member is used to specify an alternative entry
    // point that the bootloader should jump to instead of the executable's
    // ELF entry point. We do not care about that so we leave it zeroed.
    .entry_point = 0,
    // Let's tell the bootloader where our stack is.
    // We need to add the sizeof(stack) since in x86(_64) the stack grows
    // downwards.
    .stack = (uintptr_t)stack + sizeof(stack),
    // Bit 1, if set, causes the bootloader to return to us pointers in the
    // higher half, which we likely want since this is a higher half kernel.
    // Bit 2, if set, tells the bootloader to enable protected memory ranges,
    // that is, to respect the ELF PHDR mandated permissions for the executable's
    // segments.
    // Bit 3, if set, enables fully virtual kernel mappings, which we want as
    // they allow the bootloader to pick whichever *physical* memory address is
    // available to load the kernel, rather than relying on us telling it where
    // to load it.
    // Bit 4 disables a deprecated feature and should always be set.
    .flags = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4),
    // This header structure is the root of the linked list of header tags and
    // points to the first one in the linked list.
    .tags = (uintptr_t)&smp_hdr_tag};

// The following will be our kernel's entry point.
extern "C" void kernelMain(stivale2_struct *stivale2Struct)
{
    kernelInitialize(stivale2Struct);
    for (;;)
        ;
    // kernelHalt();
}

void threadProgram()
{
    logInfon("%! THREAD IS RUNNING!", "[THREAD]");
}

void userthreadMain()
{
    while (true)
        ;
}

k_ahci_driver *mainDriver;

void kernelInitialize(stivale2_struct *stivaleInfo)
{
    loggerInitialize(stivaleInfo);
    logDebugn("Logger has been initialized.");

    logDebugn("%! Stack is located at 0x%64x", "[STACK]", &stack);

    gdtInitialize();

    memoryInitialize(stivaleInfo);

    acpiInitialize(stivaleInfo);

    interruptsInitialize();

    schedulerInit();

    taskingInitialize(1);
    taskingAddCPU(0);

    filesystemInitialize();

    ELF_LOAD_STATUS status;
    if((status = elfLoad("root/apps/testapp.elf")) == SUCCESS)
        logInfon("SUCCESS");
    else 
        logInfon("FAILED %d", status);


    // k_process *proc = taskingCreateProcess();
    // taskingCreateProcess();
    // taskingCreateThread((virtual_address_t)threadProgram, proc, KERNEL);

    lapicStartTimer();
}

void kernelPanic(const char *msg, ...)
{
    logInfon("%!", "-+== KERNEL PANIC ==+-");
    va_list valist;
    va_start(valist, msg);
    loggerPrintDirect(msg, valist);
    va_end(valist);

    interruptsDisable();

    for (;;)
    {
        asm("hlt");
    }
}

void kernelHalt()
{
    interruptsDisable();

    logInfon("Kernel execution has been finished.");

    for (;;)
    {
        asm("hlt");
    }
}
