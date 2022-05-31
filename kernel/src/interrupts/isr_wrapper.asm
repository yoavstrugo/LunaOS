extern interruptHandler

global isr_wrapper
isr_wrapper:
    ; Save context
    push rbp
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    ; Store Segments
    push fs
    push gs

    xor rax, rax
    mov ax, ds
    push rax

    ; Switch to kernel space
    mov eax, 0x10 ; 0x10 is the kernel space in GDT
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    mov ss, eax

    ; Return rsp
    mov rdi, rsp
    call interruptHandler
    mov rsp, rax

    pop rax
    mov ds ,ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Retrieve Segments
    pop gs
    pop fs
    mov eax, gs
    mov ds, eax

    ; Retrieve all general purpose registers
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    pop rbp

    add rsp, 16 ; Skiping error code and interrupt number (each 8bytes)
    sti ; enable interrupts again
    iretq

%macro createExcHandler 1
global _iExc%1
_iExc%1:
	cli
	push %1
	jmp isr_wrapper
%endmacro

%macro createExcHandlerNoError 1
global _iExc%1
_iExc%1:
	cli
    push 0 ; Instead of error
	push %1
	jmp isr_wrapper
%endmacro

%macro createReqHandler 1
global _iReq%1
_iReq%1:
	cli
    push 0  ; Instead of error
	push %1
	jmp isr_wrapper
%endmacro
    
; Exceptions
createExcHandlerNoError 0       ; Divide by zero
createExcHandlerNoError 1       ; Debug
createExcHandlerNoError 2       ; Non-maskable interrupt
createExcHandlerNoError 3       ; Breakpoint
createExcHandlerNoError 4       ; Overflow
createExcHandlerNoError 5       ; Bound Range Exceeded
createExcHandlerNoError 6       ; Invalid Opcode
createExcHandlerNoError 7       ; Device Not Available
createExcHandler 8              ; Double Fault
createExcHandlerNoError 9       ; Reserved
createExcHandler 10             ; Invalid TSS
createExcHandler 11             ; Segment Not Present
createExcHandler 12             ; Stack-Segment Fault
createExcHandler 13             ; General Protection Fault
createExcHandler 14             ; Page Fault
createExcHandlerNoError 15      ; Reserved
createExcHandlerNoError 16      ; x87 Floating-Point Exception
createExcHandler 17             ; Alignment Check
createExcHandlerNoError 18      ; Machine Check
createExcHandlerNoError 19      ; SIMD Floating-Point Exception
createExcHandlerNoError 20      ; Virtualization Exception
createExcHandler 21             ; Control Protection Exception
createExcHandlerNoError 22      ; Reserved
createExcHandlerNoError 23      ; Reserved
createExcHandlerNoError 24      ; Reserved
createExcHandlerNoError 25      ; Reserved
createExcHandlerNoError 26      ; Reserved
createExcHandlerNoError 27      ; Reserved
createExcHandlerNoError 28      ; Hypervisor Injection Exception
createExcHandlerNoError 29      ; VMM Communication Exception
createExcHandlerNoError 30      ; Security Exception
createExcHandlerNoError 31      ; Reserved

; Requests
createReqHandler 32             ; Timer request
createReqHandler 33             ; Keyboard request
createReqHandler 34
createReqHandler 35
createReqHandler 36
createReqHandler 37
createReqHandler 38
createReqHandler 39
createReqHandler 40
createReqHandler 41
createReqHandler 42
createReqHandler 43
createReqHandler 44
createReqHandler 45
createReqHandler 46
createReqHandler 47
createReqHandler 48
createReqHandler 49
createReqHandler 50
createReqHandler 51
createReqHandler 52
createReqHandler 53
createReqHandler 54
createReqHandler 55
createReqHandler 56
createReqHandler 57
createReqHandler 58
createReqHandler 59
createReqHandler 60
createReqHandler 61
createReqHandler 62
createReqHandler 63
createReqHandler 64
createReqHandler 65
createReqHandler 66
createReqHandler 67
createReqHandler 68
createReqHandler 69
createReqHandler 70
createReqHandler 71
createReqHandler 72
createReqHandler 73
createReqHandler 74
createReqHandler 75
createReqHandler 76
createReqHandler 77
createReqHandler 78
createReqHandler 79
createReqHandler 80
createReqHandler 81
createReqHandler 82
createReqHandler 83
createReqHandler 84
createReqHandler 85
createReqHandler 86
createReqHandler 87
createReqHandler 88
createReqHandler 89
createReqHandler 90
createReqHandler 91
createReqHandler 92
createReqHandler 93
createReqHandler 94
createReqHandler 95
createReqHandler 96
createReqHandler 97
createReqHandler 98
createReqHandler 99
createReqHandler 100
createReqHandler 101
createReqHandler 102
createReqHandler 103
createReqHandler 104
createReqHandler 105
createReqHandler 106
createReqHandler 107
createReqHandler 108
createReqHandler 109
createReqHandler 110
createReqHandler 111
createReqHandler 112
createReqHandler 113
createReqHandler 114
createReqHandler 115
createReqHandler 116
createReqHandler 117
createReqHandler 118
createReqHandler 119
createReqHandler 120
createReqHandler 121
createReqHandler 122
createReqHandler 123
createReqHandler 124
createReqHandler 125
createReqHandler 126
createReqHandler 127
createReqHandler 128    ; System calls
createReqHandler 129
createReqHandler 130
createReqHandler 131
createReqHandler 132
createReqHandler 133
createReqHandler 134
createReqHandler 135
createReqHandler 136
createReqHandler 137
createReqHandler 138
createReqHandler 139
createReqHandler 140
createReqHandler 141
createReqHandler 142
createReqHandler 143
createReqHandler 144
createReqHandler 145
createReqHandler 146
createReqHandler 147
createReqHandler 148
createReqHandler 149
createReqHandler 150
createReqHandler 151
createReqHandler 152
createReqHandler 153
createReqHandler 154
createReqHandler 155
createReqHandler 156
createReqHandler 157
createReqHandler 158
createReqHandler 159
createReqHandler 160
createReqHandler 161
createReqHandler 162
createReqHandler 163
createReqHandler 164
createReqHandler 165
createReqHandler 166
createReqHandler 167
createReqHandler 168
createReqHandler 169
createReqHandler 170
createReqHandler 171
createReqHandler 172
createReqHandler 173
createReqHandler 174
createReqHandler 175
createReqHandler 176
createReqHandler 177
createReqHandler 178
createReqHandler 179
createReqHandler 180
createReqHandler 181
createReqHandler 182
createReqHandler 183
createReqHandler 184
createReqHandler 185
createReqHandler 186
createReqHandler 187
createReqHandler 188
createReqHandler 189
createReqHandler 190
createReqHandler 191
createReqHandler 192
createReqHandler 193
createReqHandler 194
createReqHandler 195
createReqHandler 196
createReqHandler 197
createReqHandler 198
createReqHandler 199
createReqHandler 200
createReqHandler 201
createReqHandler 202
createReqHandler 203
createReqHandler 204
createReqHandler 205
createReqHandler 206
createReqHandler 207
createReqHandler 208
createReqHandler 209
createReqHandler 210
createReqHandler 211
createReqHandler 212
createReqHandler 213
createReqHandler 214
createReqHandler 215
createReqHandler 216
createReqHandler 217
createReqHandler 218
createReqHandler 219
createReqHandler 220
createReqHandler 221
createReqHandler 222
createReqHandler 223
createReqHandler 224
createReqHandler 225
createReqHandler 226
createReqHandler 227
createReqHandler 228
createReqHandler 229
createReqHandler 230
createReqHandler 231
createReqHandler 232
createReqHandler 233
createReqHandler 234
createReqHandler 235
createReqHandler 236
createReqHandler 237
createReqHandler 238
createReqHandler 239
createReqHandler 240
createReqHandler 241
createReqHandler 242
createReqHandler 243
createReqHandler 244
createReqHandler 245
createReqHandler 246
createReqHandler 247
createReqHandler 248
createReqHandler 249
createReqHandler 250
createReqHandler 251
createReqHandler 252
createReqHandler 253
createReqHandler 254
createReqHandler 255