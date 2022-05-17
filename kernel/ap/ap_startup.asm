BITS 16
start:


BITS 64
long_mode:


gdt:
    ; Null descriptor
    dq 0x0000000000000000

    ; Kernel Code segment
    dw 0xFFFF   ; Limit Low
    dw 0x0000   ; Base Low
    db 0x00     ; Base Mid
    db 0x9A     ; Access Byte
    db 0xAF     ; Limit + Flags
    db 0x00     ; Base High

    ; Kernel Data segment
    dw 0xFFFF   ; Limit Low
    dw 0x0000   ; Base Low
    db 0x00     ; Base Mid
    db 0x92     ; Access Byte
    db 0xCF     ; Limit + Flags
    db 0x00     ; Base High

    ; User Code segment
    dw 0xFFFF   ; Limit Low
    dw 0x0000   ; Base Low
    db 0x00     ; Base Mid
    db 0xFA     ; Access Byte
    db 0xAF     ; Limit + Flags
    db 0x00     ; Base High

    ; User Data segment
    dw 0xFFFF   ; Limit Low
    dw 0x0000   ; Base Low
    db 0x00     ; Base Mid
    db 0xF2     ; Access Byte
    db 0xCF     ; Limit + Flags
    db 0x00     ; Base High