.section .text

.global _start
_start:
    mov %rsp, %rdi
    mov $main, %rsi
    call __mlibc_entry

.size _start, . - _start
