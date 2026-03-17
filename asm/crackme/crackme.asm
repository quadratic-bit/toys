.model tiny
.code
org 100h

start:
        jmp main

LOCAL_SIZE equ 32

prompt  db 'Password: $'
denied  db 13,10,'denied$'
granted db 13,10,'granted$'

password db 'secret1',0

main:
        mov dx, offset prompt
        mov ah, 09h
        int 21h

        call get_input

        mov dx, offset denied
        mov ah, 09h
        int 21h

        mov ax, 4C00h
        int 21h


get_input proc near
        push bp
        mov  bp, sp
        sub  sp, LOCAL_SIZE+2

        lea  si, [bp-LOCAL_SIZE-2]
        mov  byte ptr [si], 56
        mov  dx, si
        mov  ah, 0Ah
        int  21h

        lea  si, [bp-LOCAL_SIZE-2]

        mov  bl, [si+1]
        xor  bh, bh
        cmp  bl, LOCAL_SIZE-1
        jbe  short @F
        mov  bl, LOCAL_SIZE-1
@F:
        mov  byte ptr [si+bx+2], 0
        lea  si, [si+2]
        mov  di, offset password

cmp_loop:
        mov al, [si]
        mov bl, [di]
        cmp al, bl
        jne not_match
        cmp al, 0
        je  match
        inc si
        inc di
        jmp cmp_loop

match:
        mov sp, bp
        pop bp
        jmp win

not_match:
        mov sp, bp
        pop bp
        ret
get_input endp


win:
        mov dx, offset granted
        mov ah, 09h
        int 21h

        mov ax, 4C00h
        int 21h

end start
