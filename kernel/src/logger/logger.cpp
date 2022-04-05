#include <logger/logger.hpp>

#include <Renderer.hpp>
#include <PSF.hpp>

static PSF_Font font;
static Renderer renderer;

void loggerInitiallize(stivale2_struct_tag_framebuffer *framebuffer, stivale2_module *font_module) {
    font.initiallizePSFFont((void *)font_module->begin, (void *)font_module->end);
    renderer.initiallizeRenderer(framebuffer, &font);
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