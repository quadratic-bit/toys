; /b012345678 or /b 012345678
; border[9]
;   0 1 2    TL           T       TR
;   3 4 5    L            FILL    R
;   6 7 8    BL           B       BR

VSEG            equ 0B800h
CLR             equ 07h

MAXW            equ 40          ; max inner width excluding borders
MAXLINES        equ 32
LINESTRIDE      equ (MAXW + 1)  ; 0-terminated

BUBROW          equ 2
BUBCOL          equ 2

.model tiny
.code
org 100h

locals @@

; ------------------------------------------------------------------------------
; Display character on screen at DI
; Entry: CLR = attribute
;        AL  = character to display
;        DI  = position
; Exit:  DI  = updated position, CX decrements
; ------------------------------------------------------------------------------
PutChar MACRO
        mov     ah, CLR
        stosw
ENDM

; ------------------------------------------------------------------------------
; Display character on screen CX times, starting from DI
; Entry: CLR = attribute
;        AL  = character to display
;        CX  = times to repeat the character
;        DI  = position
; Exit:  DI  = updated position
; ------------------------------------------------------------------------------
PutRepChar MACRO
        mov     ah, CLR
        rep     stosw
ENDM

start:
        cld

        call    ParseCmdLine

        mov     ax, 0003h       ; 80x25 text mode (jk this clears the screen)
        int     10h

        mov     ax, VSEG
        mov     es, ax
        mov     ah, CLR

        call    DrawBubbleAndCow

        mov     ax, 4C00h
        int     21h

; ------------------------------------------------------------------------------
; Initialize border[] to default, extract words from remaining args and wrap.
; If no words found, use defmsg as a default
; Exit: lineCount (byte) : number of lines
;       frameW    (byte) : max line length among produced lines (<= MAXW)
;       linesbuf         : MAXLINES lines, each 0-terminated, stride LINESTRIDE
;       linelen[i]       : length of line i
;       border[9]        : border characters
; ------------------------------------------------------------------------------
ParseCmdLine proc
        push    es ; es <- ds
        push    ds
        pop     es

        mov     si, offset borderDefault
        mov     di, offset border
        mov     cx, 9
        rep     movsb

        ; copy tail to tailbuf, 0-terminated
        mov     si, 81h
        mov     cl, ds:[80h]
        xor     ch, ch
        mov     di, offset tailbuf

@@copy_tail:
        jcxz    @@tail_done
        lodsb
        cmp     al, 0Dh
        je      @@tail_done
        stosb
        loop    @@copy_tail

@@tail_done:
        mov     al, 0
        stosb

        ; wrap buffer pointers init
        mov     ax, offset linesbuf
        mov     [lineStartPtr], ax
        mov     [curPtr], ax

        ; tokenize tailbuf by spaces and tabs
        mov     si, offset tailbuf

@@next_token:
        ; skip spaces/tabs
@@skip_ws:
        mov     al, [si]
        cmp     al, 0
        jne     @@skip_ws_continue  ; damn short conditional jumps
        jmp     @@finish
@@skip_ws_continue:
        cmp     al, ' '
        je      @@ws_advance
        cmp     al, 9
        je      @@ws_advance
        jmp     @@token_start
@@ws_advance:
        inc     si
        jmp     @@skip_ws

@@token_start:
        ; tokenStart = SI, tokenEnd in DI, tokenLen in CX
        mov     bx, si                  ; BX = tokenStart
        mov     di, si
        xor     cx, cx
@@len_loop:
        mov     al, [di]
        cmp     al, 0
        je      @@len_done
        cmp     al, ' '
        je      @@len_done
        cmp     al, 9
        je      @@len_done
        inc     di
        inc     cx
        jmp     @@len_loop
@@len_done:
        ; process /b option, possibly consume this token
        cmp     [needBorder], 0
        je      @@check_b_opt

        ; needBorder==1: this token must provide 6 chars
        cmp     cx, 9
        jb      @@needborder_fail

        push    di                  ; save tokenEnd
        mov     si, bx
        mov     di, offset border
        mov     cx, 9
        rep     movsb
        pop     di                  ; restore tokenEnd

        mov     byte ptr [needBorder], 0
        jmp     @@advance_to_tokenend

@@needborder_fail:
        ; couldn't parse border, fall back to default
        mov     byte ptr [needBorder], 0
        jmp     @@message_token

@@check_b_opt:
        ; if token starts with /b or /B
        cmp     cx, 2
        jb      @@message_token
        mov     al, [bx]
        cmp     al, '/'
        jne     @@message_token
        mov     al, [bx + 1]
        cmp     al, 'b'
        je      @@b_seen
        cmp     al, 'B'
        jne     @@message_token

@@b_seen:
        cmp     cx, 2
        je      @@b_needs_next
        cmp     cx, 8
        jb      @@message_token      ; not valid /b, treat as message
        ; /bXXXXXXXXX (or longer): take next 9 bytes after 'b'
        mov     si, bx
        add     si, 2
        mov     di, offset border
        mov     cx, 9
        rep     movsb
        jmp     @@advance_to_tokenend   ; consume option token only

@@b_needs_next:
        mov     byte ptr [needBorder], 1
        jmp     @@advance_to_tokenend   ; consume /b token only

@@message_token:
        ; scan for [a-zA-Z0-9] words, append each
        mov     dx, bx                  ; DX = scan ptr
@@scan_more:
        cmp     dx, di                  ; di = tokenEnd from tokenizer
        jae     @@advance_to_tokenend

        ;mov     al, [dx] illegal stuff, only [bx] [bp] [si] [di] allowed what??
        push    si
        mov     si, dx
        mov     al, [si]
        pop     si

        call    IsWordChar
        jc      @@word_start
        inc     dx
        jmp     @@scan_more

@@word_start:
        mov     si, dx                  ; SI = word start
        xor     cx, cx
@@word_len:
        cmp     dx, di
        jae     @@have_word

        ;mov     al, [dx] illegal stuff
        push    si
        mov     si, dx
        mov     al, [si]
        pop     si

        call    IsWordChar
        jnc     @@have_word
        inc     dx
        inc     cx
        jmp     @@word_len

@@have_word:
        ; append word at DS:SI length CX
        cmp     [stopWrap], 0
        jne     @@scan_more
        push    dx                      ; save scan ptr
        call    AppendWord              ; DS:SI, CX
        pop     dx
        jmp     @@scan_more

@@advance_to_tokenend:
        ; restore SI to tokenEnd (bro is currently in DI from tokenizer)
        mov     si, di
        jmp     @@next_token

@@finish:
        ; if no words, use default message
        cmp     byte ptr [lineCount], 0
        jne     @@finalize
        mov     si, offset defmsg
        mov     cx, defmsg_len
        call    AppendWord

@@finalize:
        ; store final line length and update frameW
        call    FinalizeLastLine

        pop     es
        ret
ParseCmdLine endp

; ------------------------------------------------------------------------------
; IsWordChar
; Entry: AL = character
; Exit:  CF = 1 if [0-9A-Za-z],
;        CF = 0 otherwise
; ------------------------------------------------------------------------------
IsWordChar proc
        cmp     al, '0'
        jb      @@no
        cmp     al, '9'
        jbe     @@yes
        cmp     al, 'A'
        jb      @@check_lower
        cmp     al, 'Z'
        jbe     @@yes
@@check_lower:
        cmp     al, 'a'
        jb      @@no
        cmp     al, 'z'
        jbe     @@yes
@@no:
        clc
        ret
@@yes:
        stc
        ret
IsWordChar endp

; ------------------------------------------------------------------------------
; Append one word to wrapped text, create new lines if needed.
; Break words longer than MAXW with minus '-'.
; Entry: DS:SI = word
;        CX    = word length
; ------------------------------------------------------------------------------
AppendWord proc
        ; if no line yet, start first line
        cmp     [lineCount], 0
        jne     @@have_line
        mov     byte ptr [lineCount], 1
        mov     byte ptr [lineIdx], 0
        mov     byte ptr [curLen], 0
        mov     ax, offset linesbuf
        mov     [lineStartPtr], ax
        mov     [curPtr], ax

@@have_line:
        mov     dx, cx

        cmp     cx, MAXW
        jbe     @@normal_word

        ; long word: if current line not empty, newline first
        cmp     byte ptr [curLen], 0
        je      @@break_long
        call    NewLine
        cmp     byte ptr [stopWrap], 0
        jne     @@done

@@break_long:
        ; DX = remaining length, SI = source
@@long_loop:
        cmp     dx, MAXW
        jbe     @@long_tail

        ; write MAXW-1 chars + minus '-'
        mov     cx, (MAXW - 1)
        call    CopyToLine            ; copy CX bytes from DS:SI to current line
        mov     al, '-'
        call    PutCharToLine
        mov     byte ptr [curLen], MAXW

        call    NewLine
        cmp     [stopWrap], 0
        jne     @@done

        sub     dx, (MAXW - 1)
        jmp     @@long_loop

@@long_tail:
        ; DX <= MAXW: write remainder into current line
        mov     cx, dx
        call    CopyToLine
        mov     byte ptr [curLen], dl   ; dl holds remainder
        jmp     @@done

@@normal_word:
        ; DX = wordLen <= MAXW
        mov     al, byte ptr [curLen]
        cmp     al, 0
        je      @@copy_first

        ; need space if fits: curLen + 1 + wordLen <= MAXW
        xor     ah, ah
        mov     bx, ax                  ; BX = curLen
        mov     ax, dx                  ; AX = wordLen
        add     ax, bx
        inc     ax                      ; +1 for space
        cmp     ax, MAXW
        jbe     @@space_then_copy

        ; doesn't fit: start new line
        call    NewLine
        cmp     byte ptr [stopWrap], 0
        jne     @@done
        jmp     @@copy_first

@@space_then_copy:
        mov     al, ' '
        call    PutCharToLine
        inc     byte ptr [curLen]

@@copy_first:
        mov     cx, dx
        call    CopyToLine
        ; curLen += wordLen
        mov     al, byte ptr [curLen]
        add     al, dl
        mov     byte ptr [curLen], al

@@done:
        ret
AppendWord endp

; ------------------------------------------------------------------------------
; Copy to line
; Entry: DS:SI = source
;        CX    = count
; Exit:  advance SI
;        advance curPtr
; ------------------------------------------------------------------------------
CopyToLine proc
        push    di
        mov     di, [curPtr]
@@c_loop:
        jcxz    @@c_done
        mov     al, [si]
        mov     [di], al
        inc     si
        inc     di
        dec     cx
        jmp     @@c_loop
@@c_done:
        mov     [curPtr], di
        pop     di
        ret
CopyToLine endp

; ------------------------------------------------------------------------------
; Put char to line
; Entry: AL = char
; Exit:  increment cuPtr
; ------------------------------------------------------------------------------
PutCharToLine proc
        push    di
        mov     di, [curPtr]
        mov     [di], al
        inc     di
        mov     [curPtr], di
        pop     di
        ret
PutCharToLine endp

; ------------------------------------------------------------------------------
; Finalize current line, start next line
; ------------------------------------------------------------------------------
NewLine proc
        ; 0-terminate current line
        push    di
        mov     di, [curPtr]
        mov     byte ptr [di], 0
        pop     di

        ; store linelen[lineIdx] = curLen and update frameW
        xor     bx, bx
        mov     bl, [lineIdx]
        mov     al, [curLen]
        mov     [linelen + bx], al
        cmp     al, [frameW]
        jbe     @@no_max
        mov     [frameW], al
@@no_max:
        ; if out of lines, stop wrapping
        cmp     bl, (MAXLINES - 1)
        jb      @@has_room
        mov     byte ptr [stopWrap], 1
        ret

@@has_room:
        ; next line
        inc     bl
        mov     [lineIdx], bl
        inc     byte ptr [lineCount]

        mov     ax, [lineStartPtr]
        add     ax, LINESTRIDE
        mov     [lineStartPtr], ax
        mov     [curPtr], ax
        mov     byte ptr [curLen], 0
        ret
NewLine endp

; ------------------------------------------------------------------------------
; Store last line length and update frameW
; ------------------------------------------------------------------------------
FinalizeLastLine proc
        ; if no line, nothing
        cmp     [lineCount], 0
        je      @@done

        ; 0-terminate current line
        push    di
        mov     di, [curPtr]
        mov     byte ptr [di], 0
        pop     di

        xor     bx, bx
        mov     bl, [lineIdx]
        mov     al, [curLen]
        mov     [linelen + bx], al
        cmp     al, [frameW]
        jbe     @@done
        mov     [frameW], al
@@done:
        ret
FinalizeLastLine endp

; ------------------------------------------------------------------------------
; Draw the frame and a single spherical cow in a vacuum
; Entry: border[], frameW, lineCount, linesbuf, linelen[]
; ------------------------------------------------------------------------------
DrawBubbleAndCow proc
        ; draw top border
        mov     al, [frameW]
        xor     ah, ah
        mov     cx, ax                  ; CX = frameW
        add     cx, 2                   ; inner padding spaces left + right

        ; SetPos(BUBROW, BUBCOL) cdecl cc
        push    BUBCOL
        push    BUBROW
        call    SetPos
        add     sp, 4

        mov     al, [border + 0]          ; TL
        PutChar

        mov     al, [border + 1]          ; TOP
        PutRepChar                        ; CX times

        mov     al, [border + 2]          ; TR
        PutChar

        ; draw each text line
        xor     bx, bx                    ; line i = 0
@@line_loop:
        mov     al, [lineCount]
        xor     ah, ah
        cmp     bx, ax
        jae     @@bottom

        ; row = BUBROW + 1 + i
        mov     dx, BUBROW
        add     dx, 1
        add     dx, bx

        ; SetPos(row, BUBCOL) cdecl cc
        push    BUBCOL
        push    dx
        call    SetPos
        add     sp, 4

        mov     al, [border + 3]          ; LEFT
        PutChar
        mov     al, [border + 4]          ; FILL
        PutChar

        ; print line text
        ; SI = linesbuf + i * LINESTRIDE
        mov     si, offset linesbuf
        mov     ax, bx
        mov     dx, LINESTRIDE
        mul     dx                        ; DX:AX = i * LINESTRIDE
        add     si, ax

        mov     al, [linelen+bx]
        xor     ah, ah
        mov     cx, ax
        call    PutBufLen

        ; pad spaces: (frameW - linelen[i])
        mov     al, [frameW]
        sub     al, [linelen + bx]
        xor     ah, ah
        mov     cx, ax
        mov     al, [border + 4]          ; FILL
        PutRepChar

        mov     al, [border + 4]          ; FILL
        PutChar
        mov     al, [border + 5]          ; RIGHT
        PutChar

        inc     bx
        jmp     @@line_loop

@@bottom:
        ; draw bottom border at row = BUBROW + lineCount + 1
        mov     al, [lineCount]
        xor     ah, ah
        mov     dx, ax
        add     dx, BUBROW
        add     dx, 1

        ; SetPos(row, BUBCOL) cdecl cc
        push    BUBCOL
        push    dx
        call    SetPos
        add     sp, 4

        mov     al, [frameW]
        xor     ah, ah
        mov     cx, ax
        add     cx, 2

        mov     al, [border + 6]          ; BL
        PutChar
        mov     al, [border + 7]          ; BOTTOM
        PutRepChar
        mov     al, [border + 8]          ; BR
        PutChar

        ; cow starts at row = bottomRow + 2 = BUBROW + lineCount + 3
        mov     al, [lineCount]
        xor     ah, ah
        mov     dx, ax
        add     dx, BUBROW
        add     dx, 3                     ; cowRowStart in DX

        ; PutStrAt(row, col, pStr) pascal cc
        push    dx
        push    BUBCOL
        push    offset cow1
        call    PutStrAt

        inc     dx
        push    dx
        push    BUBCOL
        push    offset cow2
        call    PutStrAt

        inc     dx
        push    dx
        push    BUBCOL
        push    offset cow3
        call    PutStrAt

        inc     dx
        push    dx
        push    BUBCOL
        push    offset cow4
        call    PutStrAt

        inc     dx
        push    dx
        push    BUBCOL
        push    offset cow5
        call    PutStrAt

        ret
DrawBubbleAndCow endp

; ------------------------------------------------------------------------------
; cdecl
; Calculate absolute position in text video memory
; Entry: stack = col (then) row
; Exit:  DI = ((row * 80) + col) * 2
; ------------------------------------------------------------------------------
SetPos proc
        push    bp
        mov     bp, sp
        push    ax
        push    bx

        mov     bx, [bp + 4]              ; row
        mov     ax, bx
        shl     ax, 6                     ; row * 64
        mov     di, ax
        mov     ax, bx
        shl     ax, 4                     ; row * 16
        add     di, ax                    ; row * 80

        mov     ax, [bp + 6]              ; col
        add     di, ax
        shl     di, 1

        pop     bx
        pop     ax
        pop     bp
        ret                               ; YOLO, caller cleans
SetPos endp

; ------------------------------------------------------------------------------
; pascal
; Put 0-terminated pStr at (row, col)
; Entry: stack = row (then) col (then) pStr
; ------------------------------------------------------------------------------
PutStrAt proc
        push    bp
        mov     bp, sp
        push    ax
        push    bx
        push    si

        mov     si, [bp + 4]              ; pStr
        mov     bx, [bp + 6]              ; col
        mov     ax, [bp + 8]              ; row

        ; call SetPos(row, col) (cdecl)
        push    bx                        ; col
        push    ax                        ; row
        call    SetPos
        add     sp, 4
        mov     ah, CLR

@@loop:
        lodsb
        test    al, al
        jz      @@done
        stosw                             ; AL char, AH attribute
        jmp     @@loop

@@done:
        pop     si
        pop     bx
        pop     ax
        pop     bp
        ret     6                         ; 3 args
PutStrAt endp

; ------------------------------------------------------------------------------
; Print line on screen from buffer
; Entry: CLR = attribute
;        DS:SI = buffer
;        CX    = length of buffer
;        DI    = position
; Exit:  DI    = updated position
; Fucks: AX, SI
; ------------------------------------------------------------------------------
PutBufLen proc
        mov     ah, CLR
        jcxz    @@done
@@loop:
        lodsb
        stosw
        loop    @@loop
@@done:
        ret
PutBufLen endp

; ------------------------------------------------------------------------------
.data
; default border
; ╔═╗
; ║ ║
; ╚═╝
borderDefault db 0C9h, 0CDh, 0BBh, 0BAh, ' ', 0BAh, 0C8h, 0CDh, 0BCh
border        db 9 dup(0)

; wrapping state
lineCount     db 0
frameW        db 0
curLen        db 0
lineIdx       db 0
needBorder    db 0
stopWrap      db 0

lineStartPtr  dw 0
curPtr        dw 0

linelen       db MAXLINES dup(0)
linesbuf      db (MAXLINES * LINESTRIDE) dup(0)

tailbuf       db 128 dup(0)

defmsg        db 'Moov'
defmsg_len    equ ($ - defmsg)

cow1          db '     \   ^__^', 0
cow2          db '      \  (oo)\_______', 0
cow3          db '         (__)\  asm  )\/', '\', 0
cow4          db '             ||----w |', 0
cow5          db '             ||     ||', 0

end start
