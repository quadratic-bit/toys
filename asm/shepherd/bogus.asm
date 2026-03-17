.model tiny
.code
org 100h

locals @@

KBD_MASK	equ 0FDh
ESC_CODE	equ 01h

EXP_AX		equ 1111h
EXP_BX		equ 2222h
EXP_CX		equ 3333h
EXP_DX		equ 4444h
EXP_SI		equ 5555h
EXP_DI		equ 6666h
EXP_BP		equ 7777h

start:

	; load bogus funny registers
	mov     ax, EXP_AX
	mov     bx, EXP_BX
	mov     cx, EXP_CX
	mov     dx, EXP_DX
	mov     si, EXP_SI
	mov     di, EXP_DI
	mov     bp, EXP_BP

@@loop:
	cli
	in      al, 60h
	cmp     al, 1
	mov     al, 11h
	sti
	jne     @@loop

	ret


.data

end start
