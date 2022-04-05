#pragma once

#include <stivale2.h>
#include <stdint.h>
#include <PSF.hpp>
#include <stdarg.h>
#include <strings.hpp>

struct Point {
    int X, Y;
};

class Renderer {
    public:
        void initiallizeRenderer(stivale2_struct_tag_framebuffer* framebuffer, PSF_Font *font);
        void printf(const char* format, ...);
        
        // Print a character
        void putchar(unsigned short int c);    

    
    private:
        stivale2_struct_tag_framebuffer* framebuffer;
        PSF_Font *font;
        Point cursor;
        void _advance_cursor();
        void _moveCursorDown();
        void _print(const char *str);

        // internal insert a character into the given x, y position with the colors
        void _insert_char(
            /* note that this is int, not char as it's a unicode character */
            unsigned short int c,
            /* cursor position on screen, in characters not in pixels */
            int cx, int cy,
            /* foreground and background colors, say 0xFFFFFF and 0x000000 */
            uint32_t fg, uint32_t bg);

};