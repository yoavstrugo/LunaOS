#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limits.h>

struct PSF_header
{
    uint32_t magic;      /* magic bytes to identify PSF */
    uint32_t version;    /* zero */
    uint32_t headersize; /* offset of bitmaps in file, 32 */
    uint32_t flags;      /* 0 if there's no unicode table */
    uint32_t length;     /* number of glyphs */
    uint32_t charsize;   /* size of each glyph */
    uint32_t height;     /* height of each glyph in pixels */
    uint32_t width;      /* width of each glyph in pixels */
};

class PSF_Font
{
#define PSF_FONT_MAGIC 0x864ab572

public:
    void initializePSFFont(void *start_addr, void *end_addr);
    PSF_header *header();
    uint16_t *unicode();

private:
    PSF_header *_header;
    void *_start_addr;
    void *_end_addr;
    uint16_t *_unicode_ptr;
    uint16_t _unicode_table[USHRT_MAX * 2];
    void psf_init();
};
