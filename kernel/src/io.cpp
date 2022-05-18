#include <io.hpp>

void ioOutByte(uint16_t port, uint8_t value)
{
    asm volatile("out %[aPort], %[aValue]"
                 :
                 : [aPort] "dN"(port), [aValue] "a"(value));
}

uint8_t ioInByte(uint16_t port)
{
    uint8_t val;
    asm volatile("in %[aVal], %[aPort]"
                 : [aVal] "=a"(val)
                 : [aPort] "dN"(port));
    return val;
}

void ioWait()
{
    asm volatile("out 0x80, al");
}