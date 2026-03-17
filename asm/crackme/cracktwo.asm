.model tiny
.code
org 100h

locals @@

start:
        jmp main

SLOT_SIZE              equ 20
SLOT_TAG_OFF           equ 0
SLOT_USERNAME_OFF  equ 4
SLOT_PASSWORD_OFF  equ 12
SLOT_USERNAME_LEN  equ 8
SLOT_PASSWORD_LEN  equ 8

buffers:
        stage1_buf   db 16 dup(?)

desc:
        dst_slot     db 1      ; which slot second read targets
        mode         db 1      ; which handler to call later
        rec_len      db 20     ; how many bytes second read should use
        flags        db 0      ; bit0=mask second input with '*'

slots:
        slot0        label byte
        slot0_tag    db 4 dup(?)
        slot0_user   db 8 dup(?)
        slot0_pass   db 'secret69'

        slot1        label byte
        slot1_tag    db 4 dup(?)
        slot1_user   db 8 dup(?)
        slot1_pass   db 8 dup(?)

        slot2        label byte
        slot2_tag    db 4 dup(?)
        slot2_user   db 8 dup(?)
        slot2_pass   db 8 dup(?)

handlers:
        dw deny_handler
        dw verify_handler
        dw hint_handler

auth_tag db 'AUTH'

; ---------------------------------------------------------------------------
; Additional globals
; ---------------------------------------------------------------------------

stage1_len   db 0
stage2_len   db 0

msg_user         db 'Username: $'
msg_pass         db 'Password: $'
msg_granted  db 13,10,'Access granted.$'
msg_denied   db 13,10,'Access denied.$'
msg_hint         db 13,10,'Hint: username is 8 symbols, password is "AUTH" + username + password.$'
crlf             db 13,10,'$'

; ---------------------------------------------------------------------------
; Entry
;   read_stage1(stage1_buf)
;   read_stage2(slots[dst_slot], len)
;   dispatch(handlers[mode], slots[dst_slot])
; ---------------------------------------------------------------------------

main:
        push cs
        pop  ds
        push cs
        pop  es
        cld

        mov  dx, offset msg_user
        call print_dollar

        lea  dx, stage1_buf
        call read_stage1

        mov  dx, offset msg_pass
        call print_dollar

        call get_slot_ptr             ; DX = &slots[dst_slot]
        mov  cl, byte ptr [rec_len]
        call read_stage2

        call get_slot_ptr             ; DX = &slots[dst_slot]
        mov  al, byte ptr [mode]
        call dispatch

        mov  ax, 4C00h
        int  21h

print_dollar proc near
        mov  ah, 09h
        int  21h
        ret
print_dollar endp

get_slot_ptr proc near
        xor  ax, ax
        mov  al, byte ptr [dst_slot]
        cmp  al, 2
        jbe  short @@ok
        xor  al, al
@@ok:
        push bx
        mov  bl, SLOT_SIZE
        mul  bl                       ; AX = dst_slot * 20
        pop  bx
        lea  dx, slots
        add  dx, ax
        ret
get_slot_ptr endp

; IN:  DX = destination buffer
; OUT: AL = actual length typed
read_stage1 proc near
        mov  cl, 20
        xor  bh, bh                   ; echo plain text
        push 16
        call read_line
        mov  byte ptr [stage1_len], al
        ret
read_stage1 endp

; IN:  DX = destination buffer, CL = max bytes
; OUT: AL = actual length typed
read_stage2 proc near
        xor  bh, bh
        test byte ptr [flags], 1
        jz   short @@go
        mov  bh, 1                    ; masked input
@@go:
        push cx
        call read_line
        mov  byte ptr [stage2_len], al
        ret
read_stage2 endp

; DOS AH=08h for raw keyboard input; custom Enter/Backspace
; IN:
;   First stack = N bytes to clear
;   DX = destination
;   CL = max bytes
;   BH = 0 => echo chars
;   BH = 1 => echo '*'
; OUT:
;   AL = bytes read
read_line proc near
        push bp
        mov  bp, sp

        push bx
        push cx
        push dx
        push si
        push di

        mov  si, dx
        mov  di, dx

        push cx

        mov  cx, [bp+4]               ; caller-pushed N bytes to clear
        xor  al, al
        rep  stosb

        pop  cx

        xor  bl, bl                   ; BL = count

@@next:
        mov  ah, 08h
        int  21h

        cmp  al, 0Dh                  ; Enter
        je   short @@done

        cmp  al, 08h                  ; Backspace
        je   short @@backspace

        cmp  bl, cl
        jae  short @@next            ; full, ignore extras

        push bx
        xor  bh, bh
        mov  [si+bx], al
        pop  bx
        inc  bl

        cmp  bh, 0
        jne  short @@masked_echo

        mov  dl, al
        mov  ah, 02h
        int  21h
        jmp  short @@next

@@masked_echo:
        mov  dl, '*'
        mov  ah, 02h
        int  21h
        jmp  short @@next

@@backspace:
        cmp  bl, 0
        je   short @@next

        dec  bl
        push bx
        xor  bh, bh
        mov  byte ptr [si+bx], 0
        pop  bx

        mov  dl, 08h
        mov  ah, 02h
        int  21h
        mov  dl, ' '
        int  21h
        mov  dl, 08h
        int  21h
        jmp  short @@next

@@done:
        mov  dx, offset crlf
        mov  ah, 09h
        int  21h

        mov  al, bl

        pop  di
        pop  si
        pop  dx
        pop  cx
        pop  bx
        pop  bp
        ret  2
read_line endp

; IN:
;   AL = mode
;   DX = pointer to selected slot
dispatch proc near
        cmp  al, 2
        jbe  short @@ok
        xor  al, al                   ; default to deny_handler
@@ok:
        xor  ah, ah
        shl  ax, 1
        mov  bx, ax
        mov  si, dx                   ; handler argument = slots[dst_slot]
        call word ptr [handlers+bx]
        ret
dispatch endp

; ---------------------------------------------------------------------------
; Handlers
; SI points at slots[dst_slot]
; ---------------------------------------------------------------------------

deny_handler proc near
        mov  dx, offset msg_denied
        call print_dollar
        ret
deny_handler endp

verify_handler proc near
        push si

        ; require stage2_len == rec_len
        mov  al, byte ptr [stage2_len]
        cmp  al, byte ptr [rec_len]
        jne  short vh_fail_pop

        ; require tag == 'AUTH'
        lea  si, [si+SLOT_TAG_OFF]
        lea  di, auth_tag
        mov  cx, 4
        repe cmpsb
        jne  short vh_fail_pop

        ; compare stage1_buf to slot.username
        pop  si
        push si
        lea  si, [si+SLOT_USERNAME_OFF]
        lea  di, stage1_buf
        mov  cx, SLOT_USERNAME_LEN
        repe cmpsb
        jne  short vh_fail_pop

        ; compare slot.password to expected password
        pop  si
        push si
        lea  si, [si+SLOT_PASSWORD_OFF]
        lea  di, slot0_pass
        mov  cx, 8
        repe cmpsb
        jne  short vh_fail_pop

        pop  si
        mov  dx, offset msg_granted
        call print_dollar
        ret

vh_fail_pop:
        pop  si
        mov  dx, offset msg_denied
        call print_dollar
        ret
verify_handler endp

hint_handler proc near
        mov  dx, offset msg_hint
        call print_dollar
        ret
hint_handler endp

end start
