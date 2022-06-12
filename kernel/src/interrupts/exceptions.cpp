#include <interrupts/exceptions.hpp>

#include <kernel.hpp>
#include <logger/logger.hpp>

void (*exceptionHandlers[32])(uint64_t);

void exceptionDoubleFault(uint64_t code)
{
    kernelPanic("%! A double fault has occurred!", "Memory Exception:");
}

void exceptionGPFault(uint64_t code)
{
    kernelPanic("%! A general protection fault has occurred with error code %d:\
                \n\t- External Event: %s\
                \n\t- Descriptor Location: %s\
                \n\t- GDT/LDT: %s\
                \n\t- SS Index: %d!", 
                "Memory Exception:", 
                code,
                (code & 0b1) ? "true" : "false", 
                (code & 0b10) ? "true" : "false",
                (code & 0b100) ? "true" : "false",
                (code >> 3) & 0xfff8);
}

struct stackframe {
  struct stackframe* rbp;
  uint64_t rip;
};
void TraceStackTrace(unsigned int maxFrames)
{
    stackframe *stk;
    asm ("movq %%rbp,%0" : "=r"(stk) ::);
    logDebugn("Stack trace:\n");
    for(unsigned int frame = 0; stk && frame < maxFrames; ++frame)
    {
        // Unwind to previous stack frame
        logDebugn("  0x%64x     \n", stk->rip);
        stk = stk->rbp;
    }
}

void exceptionPageFault(uint64_t code)
{
    bool P = code  & 1 << 0;
    bool WR = code & 1 << 1;
    bool US = code & 1 << 2;
    bool RSVD = code & 1 << 3;
    bool ID = code & 1 << 4;
    bool PK = code & 1 << 5;
    bool SS = code & 1 << 6;
    bool HLAT = code & 1 << 7;
    bool SGX = code & 1 << 15;
    virtual_address_t relevantAddress;
    // The address the page fault has occurred at is in CR2
    asm volatile ("mov %[aRelevantAddress], cr2" : [aRelevantAddress]"=r"(relevantAddress));

    // TraceStackTrace(5);

    kernelPanic("%! A page fault has occurred with code %d on address 0x%64x!\
    \n\t- Reason: %s\
    \n\t- When: %s\
    \n\t- Who: %s\
    \n\t- Reserved? %s\
    \n\t- Instruction Fetch? %s\
    \n\t- Protection Key? %s\
    \n\t- Shadow Stack? %s\
    \n\t- During HLAT? %s\
    \n\t- SGX Related? %s", 
    "Memory Exception:", code, relevantAddress,
    P ? "Page-level protection violation" : "Non-present page access",
    WR ? "During write" : "During read",
    US ? "User-mode" : "Supervisor-mode",
    RSVD ? "yes" : "no",
    ID ? "yes" : "no",
    PK ? "yes" : "no",
    SS ? "yes" : "no",
    HLAT ? "yes" : "no",
    SGX ? "yes" : "no"
    );
    
}
