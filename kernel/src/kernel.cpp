#include <stdint.h>
#include <stddef.h>
#include <stivale2.h>
#include <strings.hpp>
#include <Renderer.hpp>
#include <PSF.hpp>
// #include <memory/PhysicalMemoryAllocator.hpp>
#include <memory/PhysicalMemoryManager.hpp>
#include <memory/BitmapAllocator.hpp>
#include <memory/VirtualMemoryManager.hpp>
#include <memory/Memory.hpp>

// We need to tell the stivale bootloader where we want our stack to be.
// We are going to allocate our stack as an array in .bss.
static uint8_t stack[0x100000];

// stivale2 uses a linked list of tags for both communicating TO the
// bootloader, or receiving info FROM it. More information about these tags
// is found in the stivale2 specification.

// stivale2 offers a runtime terminal service which can be ditched at any
// time, but it provides an easy way to print out to graphical terminal,
// especially during early boot.
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    // All tags need to begin with an identifier and a pointer to the next tag.
    .tag = {
        // Identification constant defined in stivale2.h and the specification.
        .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
        // If next is 0, it marks the end of the linked list of header tags.
        .next = 0
    },
    // The terminal header tag possesses a flags field, leave it as 0 for now
    // as it is unused.
    .flags = 0,
    .callback = 0
};


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
        .next = 0//(uint64_t)&terminal_hdr_tag
    },
    // We set all the framebuffer specifics to 0 as we want the bootloader
    // to pick the best it can.
    .framebuffer_width  = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp    = 0,
    .unused = 0
};

// The stivale2 specification says we need to define a "header structure".
// This structure needs to reside in the .stivale2hdr ELF section in order
// for the bootloader to find it. We use this __attribute__ directive to
// tell the compiler to put the following structure in said section.
__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr = {
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
    .tags = (uintptr_t)&framebuffer_hdr_tag
};

// We will now write a helper function which will allow us to scan for tags
// that we want FROM the bootloader (structure tags).
void *stivale2_get_tag(stivale2_struct *stivale2_struct, uint64_t id) {
    stivale2_tag *current_tag = (stivale2_tag *)stivale2_struct->tags;
    for (;;) {
        // If the tag pointer is NULL (end of linked list), we did not find
        // the tag. Return NULL to signal this.
        if (current_tag == NULL) {
            return NULL;
        }

        // Check whether the identifier matches. If it does, return a pointer
        // to the matching tag.
        if (current_tag->identifier == id) {
            return current_tag;
        }

        // Get a pointer to the next tag in the linked list and repeat.
        current_tag = (stivale2_tag *)current_tag->next;
    }
}
 

stivale2_module *stivale2_get_module(stivale2_struct_tag_modules *modules_struct, const char *module_string) {
    stivale2_module *current_module = modules_struct->modules;

    for (;;) {
        if (current_module == NULL) return NULL;

        if (strcmp(current_module->string, module_string) == 0)
            return current_module;

        current_module++;
    }
}



// The following will be our kernel's entry point.
extern "C" void _start(struct stivale2_struct *stivale2_struct) {
    stivale2_struct_tag_framebuffer *framebuffer = (stivale2_struct_tag_framebuffer *)stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);

    stivale2_struct_tag_modules *modules = (stivale2_struct_tag_modules *)stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MODULES_ID);
    stivale2_module *font_module = stivale2_get_module(modules, "psf_font");

    PSF_Font font = PSF_Font((void *)font_module->begin, (void *)font_module->end);
    Renderer renderer = Renderer(framebuffer, &font);

    stivale2_struct_tag_memmap *memmap_struct = (stivale2_struct_tag_memmap *)stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MEMMAP_ID);

    if (memmap_struct != NULL) renderer.printf("Found the memap struct!\n");

    BitmapAllocator bitmapAllocator = BitmapAllocator(memmap_struct);
    PhysicalMemoryManager *pmm = &bitmapAllocator;

    physical_address addr = pmm->allocateBlock();

    physical_address PML4address;
    asm volatile("mov %%cr3, %0" : "=r"(PML4address));

    renderer.printf("PML4 address -  0x%64x", PML4address);
 
    VirtualMemoryManager VMM = VirtualMemoryManager(PML4address, pmm);
    
    VMM.mapPage(0xCCCCCCCCCCCCCCCC, 0x5000);
    VMM.unmapPage(0xCCCCCCCCCCCCCCCC);

    // We're done, just hang...
    for (;;) {
        asm ("hlt");
    }
}