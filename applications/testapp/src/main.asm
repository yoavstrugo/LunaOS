BITS 64
section .data
callData db "This has been printed from a thread!", 10, 0

section .text
global mainasm
mainasm:
    mov rax, 0x1
    mov rbx, callData
    int 0x80

    mov rax, 0x0
    int 0x80