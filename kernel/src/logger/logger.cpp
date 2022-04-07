#include <logger/logger.hpp>

#include <Renderer.hpp>
#include <PSF.hpp>
#include <stivale2/stivale2_tools.hpp>

static PSF_Font font;
static Renderer renderer;

void loggerInitiallize(stivale2_struct *stivaleInfo) {
    stivale2_struct_tag_framebuffer *framebuffer = (stivale2_struct_tag_framebuffer *)stivale2_get_tag(stivaleInfo, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);

    stivale2_struct_tag_modules *modules = (stivale2_struct_tag_modules *)stivale2_get_tag(stivaleInfo, STIVALE2_STRUCT_TAG_MODULES_ID);
    stivale2_module *fontModule = stivale2_get_module(modules, "psf_font");

    font.initiallizePSFFont((void *)fontModule->begin, (void *)fontModule->end);
    renderer.initiallizeRenderer(framebuffer, &font);
}

void loggerPrintDirect(const char *msg, va_list valist) {
    renderer.printf(msg, valist);
}

void loggerPrint(const char *msg, ...) {
    va_list valist;
    va_start(valist, msg);
    renderer.printf(msg, valist);
    va_end(valist);
}

void loggerPrintln(const char *msg, ...) {
    va_list valist;
    va_start(valist, msg);
    renderer.printf(msg, valist);
    renderer.putchar('\n');
    va_end(valist);
}