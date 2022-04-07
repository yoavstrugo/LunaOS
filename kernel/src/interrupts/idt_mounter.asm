BITS 64

global _loadIDT

;
; this is the extern function for
; void _loadIDT(k_idt_descriptor *idtDescriptorAddress)
;
; Parameters:
;   rdi     idtDescriptorAddress
;
_loadIDT:
    lidt [rdi]
    ret