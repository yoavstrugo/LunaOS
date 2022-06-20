#include <Renderer.hpp>

#include <strings.hpp>
#include <memory/memory.hpp>

#define PIXEL uint32_t /* pixel pointer */
#define TABSIZE 4

void Renderer::initializeRenderer(stivale2_struct_tag_framebuffer *framebuffer, PSF_Font *font)
{
    this->framebuffer = framebuffer;
    this->font = font;

    this->cursor.X = 0;
    this->cursor.Y = 0;

    this->defaultColor();
}

void Renderer::scrollUp()
{
#define BPP this->framebuffer->framebuffer_bpp / 8
#define WIDTH this->framebuffer->framebuffer_width
#define LINE_HEIGHT this->font->header()->height
#define HEIGHT this->framebuffer->framebuffer_height
#define ADDR this->framebuffer->framebuffer_addr
    memcpy(
        (void *)ADDR,
        (void *)(ADDR + WIDTH * BPP * LINE_HEIGHT),
        WIDTH * BPP * HEIGHT);
    this->cursor.Y -= this->font->header()->height;
}

void Renderer::backspace()
{
    // Check special cases
    this->cursor.X -= this->font->header()->width;
    this->_insert_char(' ', this->cursor.X, this->cursor.Y);
}

void Renderer::printf(const char *format, va_list arg)
{
    uint64_t i;
    char *s;

    for (char *traverse = (char *)format; *traverse != '\0'; traverse++)
    {
        // Print all chracters until we get a '%'
        while (*traverse != '%' && *traverse != '\0')
        {
            putchar(*traverse);
            traverse++;
        }

        if (*traverse == '\0')
            break;

        traverse++;

        // After a '%', check the next character
        switch (*traverse)
        {
        case 'c':
            i = va_arg(arg, int); // char argument
            putchar(i);
            break;
        case 'd':
            i = va_arg(arg, int);
            _print(toString(i));
            break;
        case 's':
            s = va_arg(arg, char *);
            _print((const char *)s);
            break;
        case 'x':
            i = va_arg(arg, uint64_t);
            _print(toHex((uint64_t)i, true));
            break;
        case 'm':
            i = va_arg(arg, uint64_t);
            _print(toString(K_MEMORY_SIZE(i)));
            _print(K_MEMORY_UNIT(i));
            break;
        case '!':
            s = va_arg(arg, char *);
            this->setColor(K_HEADER_FG, K_HEADER_BG);
            _print((const char *)s);
            this->defaultColor();
            break;
        default:
            i = va_arg(arg, uint64_t);
            if (!strncmp((const char *)traverse, "8x", 2))
            {
                traverse++;
                _print(toHex((uint8_t)i));
            }
            else if (!strncmp((const char *)traverse, "16x", 3))
            {
                traverse += 2;
                _print(toHex((uint16_t)i));
            }
            else if (!strncmp((const char *)traverse, "32x", 3))
            {
                traverse += 2;
                _print(toHex((uint32_t)i));
            }
            else if (!strncmp((const char *)traverse, "64x", 3))
            {
                traverse += 2;
                _print(toHex((uint64_t)i));
            }
        }
    }
}

void Renderer::_print(const char *str)
{
    // get a pointer to the first char
    char *chr = (char *)str;
    while (*chr != 0)
    {
        // write the char and increase the cursor
        putchar(*chr);
        chr++;
    }
}

void Renderer::_advanceCursor()
{
    this->cursor.X += this->font->header()->width;
    if (this->cursor.X >= this->framebuffer->framebuffer_width)
    {
        this->cursor.X = 0;
        this->cursor.Y += this->font->header()->height;

        if (this->cursor.Y + this->font->header()->height >= this->framebuffer->framebuffer_height)
        {
            scrollUp();
            this->cursor.Y -= this->font->header()->height;
        }
    }
}

void Renderer::putchar(unsigned short int c)
{
    // Check special cases
    switch (c)
    {
    case '\n':
        // New line
        this->cursor.Y += this->font->header()->height;
        this->cursor.X = 0;
        if (this->cursor.Y + this->font->header()->height >= this->framebuffer->framebuffer_height)
        {
            scrollUp();
        }
        return;
    case 0x8:
        this->backspace();
        return;
    case '\t':
        // Horizontal tab
        for (int i = 0; i < TABSIZE; i++)
            this->_advanceCursor();
        return;
    }

    this->_insert_char(c, this->cursor.X, this->cursor.Y);
    this->_advanceCursor();
}

void Renderer::defaultColor()
{
    this->setColor(0xFFFFFF, 0x000000);
}

void Renderer::setColor(uint32_t fg, uint32_t bg)
{
    this->_fg = fg;
    this->_bg = bg;
}

void Renderer::_insert_char(
    /* note that this is int, not char as it's a unicode character */
    unsigned short int c,
    /* cursor position on screen, in characters not in pixels */
    int cx, int cy)
{

    uint16_t *unicode = this->font->unicode();

    /* we need to know how many bytes encode one row */
    // int bytesperline = (this->font->header()->width + 7) / 8;

    /* unicode translation */
    if (unicode != NULL)
    {
        c = unicode[c];
    }

    uint64_t fontOffset = (uint64_t)font->header() + font->header()->headersize;

    /* get the glyph for the character. If there's no
       glyph for a given character, we'll display the first glyph. */
    unsigned char *glyph = (unsigned char *)(fontOffset +
                                             (c > 0 && c < font->header()->length ? c : 0) * font->header()->charsize);

    /* calculate the upper left corner on screen where we want to display.
       we only do this once, and adjust the offset later. This is faster. */
    uint32_t initial_offset = (cy * framebuffer->framebuffer_width + cx) * sizeof(PIXEL);

    /* finally display pixels according to the bitmap */
    uint32_t current_offset;
    uint16_t mask;
    uint64_t fbAddress = this->framebuffer->framebuffer_addr;

    for (uint32_t y = 0; y < this->font->header()->height; y++)
    {
        /* save the starting position of the line */
        current_offset = initial_offset;
        mask = 1 << (16 - 1);

        uint16_t actual_glyph = ((*((uint16_t *)glyph)) << 8) | ((*(uint16_t *)glyph) >> 8);
        /* display a row */
        for (uint32_t x = 0; x < this->font->header()->width; x++)
        {
            *((PIXEL *)(fbAddress + current_offset)) = actual_glyph & mask ? this->_fg : this->_bg;
            /* adjust to the next pixel */
            mask >>= 1;
            current_offset += sizeof(PIXEL);
        }
        /* adjust to the next line */
        glyph += font->header()->charsize / font->header()->height;
        initial_offset += this->framebuffer->framebuffer_width * sizeof(PIXEL);
    }
}