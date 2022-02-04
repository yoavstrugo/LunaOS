#include "PSF.h"

PSF_Font::PSF_Font(void *start_addr, void *end_addr) {
    this->_start_addr = start_addr;
    this->_end_addr = end_addr;
    this->_header = (PSF_header*)start_addr;

    this->psf_init();
}

PSF_header *PSF_Font::header() {
    return this->_header;
}

uint16_t *PSF_Font::unicode() {
    return this->_unicode_ptr;
}

void PSF_Font::psf_init()
{
    uint16_t glyph = 0;

    /* cast the address to PSF header struct */
    
    /* is there a unicode table? */
    if (this->_header->flags) {
        this->_unicode_ptr = NULL;
        return; 
    }

    this->_unicode_ptr = this->_unicode_table;
    /* get the offset of the table */
    char *s = (char *)(
    (unsigned char*)this->_start_addr +
      this->_header->headersize +
      this->_header->length * this->_header->charsize
    );
    
    while(s>_end_addr) {
        uint16_t uc = (uint16_t)(((unsigned char *)s)[0]);
        if(uc == 0xFF) {
            glyph++;
            s++;
            continue;
        } else if(uc & 128) {
            /* UTF-8 to unicode */
            if((uc & 32) == 0 ) {
                uc = ((s[0] & 0x1F)<<6)+(s[1] & 0x3F);
                s++;
            } else
            if((uc & 16) == 0 ) {
                uc = ((((s[0] & 0xF)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F);
                s+=2;
            } else
            if((uc & 8) == 0 ) {
                uc = ((((((s[0] & 0x7)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F))<<6)+(s[3] & 0x3F);
                s+=3;
            } else
                uc = 0;
        }
        /* save translation */
        _unicode_ptr[uc] = glyph;
        s++;
    }
}