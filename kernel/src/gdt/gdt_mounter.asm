BITS 64
global _loadGDT


;
; the is the extern function for
; void _loadGDT(k_gdt_descriptor *gdtDescriptorAddress)
;
_loadGDT:
    lgdt [rdi]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    pop rdi
    mov rax, 0x08
    push rax
    push rdi
    retfq

global _loadTSS
_loadTSS:
    mov ax, di
	ltr ax
	ret