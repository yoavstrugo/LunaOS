#pragma once

#include <stivale2.h>
#include <stdint.h>
#include <PSF.hpp>
#include <stdarg.h>

#define K_HEADER_FG 0xF85E2B
#define K_HEADER_BG 0x000000

typedef uint32_t color_t;

struct Point
{
    int X, Y;
};

class Renderer
{
public:
    void initializeRenderer(stivale2_struct_tag_framebuffer *framebuffer, PSF_Font *font);
    void printf(const char *format, va_list arg);

    // Print a character
    void putchar(unsigned short int c);
    void setColor(color_t fg, color_t bg);
    void defaultColor();

private:
    stivale2_struct_tag_framebuffer *framebuffer;
    PSF_Font *font;
    Point cursor;

    color_t _fg;
    color_t _bg;

    void scrollUp();
    void _advanceCursor();
    void _moveCursorDown();
    void _print(const char *str);

    // internal insert a character into the given x, y position with the colors
    void _insert_char(
        /* note that this is int, not char as it's a unicode character */
        unsigned short int c,
        /* cursor position on screen, in characters not in pixels */
        int cx, int cy);
};