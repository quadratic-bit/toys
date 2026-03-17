.model tiny
.code
org 100h

locals @@

VSEG            equ 0B800h
CLR             equ 07h
CLR_GREEN       equ 82h
CLR_BROWN       equ 86h

ROWBYTES        equ 160         ; 80 cols * 2 bytes per cell

BOX_W           equ 28
BOX_H           equ 10

COW_H           equ 5
COW_GAP         equ 1

; whole restorable area:
; box + empty gap row + cow rows
SAVE_W          equ BOX_W
SAVE_H          equ (BOX_H + COW_GAP + COW_H)
SAVE_SKIP       equ (ROWBYTES - (SAVE_W * 2))

POOP_W          equ 4
POOP_ROW        equ (BOX_H + COW_GAP + 3)

; when flipped, poop goes on the left, so keep enough margin
MIN_BOX_COL     equ POOP_W
MAX_BOX_COL     equ (80 - SAVE_W - POOP_W)
MAX_BOX_ROW     equ (25 - SAVE_H)

BOX_COL_RANGE   equ (MAX_BOX_COL - MIN_BOX_COL + 1)

; borders ╔╗╚╝═║
CH_TL           equ 0C9h
CH_TR           equ 0BBh
CH_BL           equ 0C8h
CH_BR           equ 0BCh
CH_H            equ 0CDh
CH_V            equ 0BAh

; ------------------------------------------------------------------------------
; Display character on screen at DI
; Entry: CLR = attribute
;        AL  = character to display
;        DI  = position
; Exit:  DI  = updated position
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
LOCAL @@loop, @@done
        mov     ah, CLR
        jcxz    @@done
@@loop:
        stosw
        loop    @@loop
@@done:
ENDM

; ------------------------------------------------------------------------------
; Put hexadecimal digit in AL to screen.
; Entry: AL = nibble 0..15
; ------------------------------------------------------------------------------
PutHexNib MACRO
LOCAL @@digit, @@out
        cmp     al, 9
        jbe     @@digit
        add     al, ('A' - 10)
        jmp     @@out
@@digit:
        add     al, '0'
@@out:
        PutChar
ENDM

; ------------------------------------------------------------------------------
; Put hexadecimal byte in AL to screen.
; ------------------------------------------------------------------------------
PutHex8 MACRO
        push    ax
        push    bx

        mov     bl, al

        mov     al, bl
        shr     al, 4
        and     al, 0Fh
        PutHexNib

        mov     al, bl
        and     al, 0Fh
        PutHexNib

        pop     bx
        pop     ax
ENDM

; ------------------------------------------------------------------------------
; Put hexadecimal word in AX to screen.
; ------------------------------------------------------------------------------
PutHex16 MACRO
        push    ax
        mov     al, ah
        PutHex8
        pop     ax
        PutHex8
ENDM

; ------------------------------------------------------------------------------
; Put character while preserving full existing attribute.
; Entry: AL    = character
;        ES:DI = screen cell
; Exit:  DI    = updated position
; ------------------------------------------------------------------------------
PutCharKeepAttr MACRO
        mov     es:[di], al
        add     di, 2
ENDM

; ------------------------------ START -----------------------------------------

start:
        ; old 09h interrupt vector
        mov     ax, 3509h
        int     21h               ; DOS Fn 35H: Get Interrupt Vector, AL int
                                  ; ES:BX address of the interrupt handler
        mov     [OldOffset], bx
        mov     [OldSegment], es

        mov     dx, offset Handler
        mov     ax, 2509h         ; DOS Fn 25H: Set Interrupt Vector, AL int
                                  ; DS:DX interrupt vector
        int     21h

        ; resident evil
        mov     dx, offset EndResident
        add     dx, 0Fh           ; for a good measure
        shr     dx, 4             ; bytes -> paragraphs
        mov     ax, 3100h
        int     21h               ; DOS Fn 31H: Terminate & Stay Resident
                                  ; DX = memory size to keep resident

Handler proc far
        ; my precious registers
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        push    bp
        push    ds
        push    es

        mov     bp, sp

        push    cs
        pop     ds

        ; [bp+00] ES
        ; [bp+02] DS
        ; [bp+04] BP
        ; [bp+06] DI
        ; [bp+08] SI
        ; [bp+0A] DX
        ; [bp+0C] CX
        ; [bp+0E] BX
        ; [bp+10] AX
        ; [bp+12] IP
        ; [bp+14] CS
        ; [bp+16] FLAGS

        mov     ax, [bp+10h]
        mov     [RegAX], ax
        mov     ax, [bp+0Eh]
        mov     [RegBX], ax
        mov     ax, [bp+0Ch]
        mov     [RegCX], ax
        mov     ax, [bp+0Ah]
        mov     [RegDX], ax
        mov     ax, [bp+08h]
        mov     [RegSI], ax
        mov     ax, [bp+06h]
        mov     [RegDI], ax
        mov     ax, [bp+04h]
        mov     [RegBP], ax
        mov     ax, [bp+02h]
        mov     [RegDS], ax
        mov     ax, [bp+00h]
        mov     [RegES], ax

        mov     ax, [bp+12h]
        mov     [RegIP], ax
        mov     ax, [bp+14h]
        mov     [RegCS], ax
        mov     ax, [bp+16h]
        mov     [RegFL], ax

        mov     ax, ss
        mov     [RegSS], ax
        lea     ax, [bp+12h]
        mov     [RegSP], ax

        ; scancode at port 60h
        in      al, 60h
        mov     [LastScan], al

        cmp     al, 058h          ; F12
        je      @@hotkey

        ; no F12 :sadge: restore pushes and jump to old ISR
        pop     es
        pop     ds
        pop     bp
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        jmp     dword ptr cs:[OldOffset] ; mega jump to old ISR

@@hotkey:
        pushf
        cli
        call    ToggleBox
        popf

        ; 61h pulse
        in      al, 61h
        mov     ah, al
        or      al, 80h
        out     61h, al
        mov     al, ah
        out     61h, al

        ; End of Interrupt
        mov     al, 20h           ; Non-specific EOI
        out     20h, al

        pop     es
        pop     ds
        pop     bp
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        iret
Handler endp

; -------------------------- toggle / draw -------------------------------------

; ------------------------------------------------------------------------------
; Toggle overlay on F12.
; If already visible, restore original screen area.
; If hidden, compute placement, save background, then draw frame + cow + something
; ------------------------------------------------------------------------------
ToggleBox proc
        cmp     byte ptr [BoxOn], 0
        je      @@on

        call    RestoreBoxArea
        mov     byte ptr [BoxOn], 0
        ret

@@on:
        call    ComputePlacement
        call    SaveBoxArea
        call    DrawRegsBox
        call    DrawCow
        call    DrawPoop
        mov     byte ptr [BoxOn], 1
        ret
ToggleBox endp

; ------------------------------------------------------------------------------
; Compute random placement and orientation.
; Exit: BoxRow  = top row of box
;       BoxCol  = left column of box
;       CowFlip = 0 normal, 1 flipped
; ------------------------------------------------------------------------------
ComputePlacement proc
        push    ax
        push    bx
        push    dx

        ; -------- row --------
        mov     ax, [RegAX]
        xor     ax, [RegBX]
        add     ax, [RegCX]
        xor     ax, [RegDX]
        add     ax, [RegSI]
        xor     ax, [RegDI]
        add     ax, [RegBP]
        xor     ax, [RegIP]
        add     ax, [RegFL]
        xor     ax, [RegSP]

        xor     dx, dx
        mov     bx, (MAX_BOX_ROW + 1)
        div     bx
        mov     [BoxRow], dl

        ; -------- flip --------
        mov     ax, [RegAX]
        xor     ax, [RegBX]
        xor     ax, [RegCX]
        xor     ax, [RegDX]
        xor     ax, [RegSI]
        xor     ax, [RegDI]
        xor     ax, [RegBP]
        xor     ax, [RegSP]
        xor     ax, [RegIP]
        xor     ax, [RegFL]
        and     al, 1
        mov     [CowFlip], al

        ; -------- column --------
        mov     ax, [RegCS]
        xor     ax, [RegDS]
        add     ax, [RegES]
        xor     ax, [RegSS]
        add     ax, [RegBX]
        xor     ax, [RegCX]
        add     ax, [RegDX]
        xor     ax, [RegSI]
        xchg    al, ah
        xor     ax, [RegFL]

        xor     dx, dx
        mov     bx, BOX_COL_RANGE
        div     bx
        add     dl, MIN_BOX_COL
        mov     [BoxCol], dl

        pop     dx
        pop     bx
        pop     ax
        ret
ComputePlacement endp

; ------------------------------------------------------------------------------
; Calculate absolute position of box in text video memory.
; Uses BoxRow / BoxCol from resident data.
; Exit: DI = ((BoxRow * 80) + BoxCol) * 2
; ------------------------------------------------------------------------------
CalcBoxOff proc
        push    ax
        push    bx

        xor     bx, bx
        mov     bl, cs:[BoxRow]

        mov     ax, bx
        shl     ax, 6             ; row * 64
        mov     di, ax
        mov     ax, bx
        shl     ax, 4             ; row * 16
        add     di, ax            ; row * 80

        xor     ax, ax
        mov     al, cs:[BoxCol]
        add     di, ax
        shl     di, 1

        pop     bx
        pop     ax
        ret
CalcBoxOff endp

; ------------------------------------------------------------------------------
; Save rectangular screen area that will be covered by box + cow.
; Copies from video memory into SavedArea.
; ------------------------------------------------------------------------------
SaveBoxArea proc
        push    ax
        push    bx
        push    cx
        push    si
        push    di
        push    ds
        push    es
        cld

        call    CalcBoxOff        ; DS still points to resident segment here
        mov     si, di

        mov     ax, VSEG
        mov     ds, ax            ; DS = video
        mov     ax, cs
        mov     es, ax            ; ES = our segment

        mov     di, offset SavedArea
        mov     bx, SAVE_H

@@row:
        mov     cx, SAVE_W
        rep     movsw
        add     si, SAVE_SKIP
        dec     bx
        jnz     @@row

        pop     es
        pop     ds
        pop     di
        pop     si
        pop     cx
        pop     bx
        pop     ax
        ret
SaveBoxArea endp

; ------------------------------------------------------------------------------
; Restore previously saved screen area from SavedArea back to video memory.
; ------------------------------------------------------------------------------
RestoreBoxArea proc
        push    ax
        push    bx
        push    cx
        push    si
        push    di
        push    ds
        push    es
        cld

        mov     ax, cs
        mov     ds, ax            ; DS = our segment
        mov     ax, VSEG
        mov     es, ax            ; ES = video

        mov     si, offset SavedArea
        call    CalcBoxOff
        mov     bx, SAVE_H

@@row:
        mov     cx, SAVE_W
        rep     movsw
        add     di, SAVE_SKIP
        dec     bx
        jnz     @@row

        pop     es
        pop     ds
        pop     di
        pop     si
        pop     cx
        pop     bx
        pop     ax
        ret
RestoreBoxArea endp

; ------------------------------------------------------------------------------
; Draw framed register monitor box at current placement.
; ------------------------------------------------------------------------------
DrawRegsBox proc
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        push    es
        cld

        mov     ax, VSEG
        mov     es, ax
        call    CalcBoxOff
        mov     si, di

        ; top border
        mov     di, si
        mov     al, CH_TL
        PutChar
        mov     al, CH_H
        mov     cx, (BOX_W - 2)
        PutRepChar
        mov     al, CH_TR
        PutChar

        ; inner lines (blank)
        mov     bx, (BOX_H - 2)
        mov     di, si
        add     di, ROWBYTES

@@inner:
        push    di
        mov     al, CH_V
        PutChar
        mov     al, ' '
        mov     cx, (BOX_W - 2)
        PutRepChar
        mov     al, CH_V
        PutChar
        pop     di
        add     di, ROWBYTES
        dec     bx
        jnz     @@inner

        ; bottom border
        mov     di, si
        add     di, (ROWBYTES * (BOX_H - 1))
        mov     al, CH_BL
        PutChar
        mov     al, CH_H
        mov     cx, (BOX_W - 2)
        PutRepChar
        mov     al, CH_BR
        PutChar

        ; write register lines inside (col 2 inside box, +4 bytes)
        ; line 1: AX BX
        mov     di, si
        add     di, (ROWBYTES * 1) + 4
        call    Print_AX_BX
        ; line 2: CX DX
        mov     di, si
        add     di, (ROWBYTES * 2) + 4
        call    Print_CX_DX
        ; line 3: SI DI
        mov     di, si
        add     di, (ROWBYTES * 3) + 4
        call    Print_SI_DI
        ; line 4: BP SP
        mov     di, si
        add     di, (ROWBYTES * 4) + 4
        call    Print_BP_SP
        ; line 5: CS DS
        mov     di, si
        add     di, (ROWBYTES * 5) + 4
        call    Print_CS_DS
        ; line 6: ES SS
        mov     di, si
        add     di, (ROWBYTES * 6) + 4
        call    Print_ES_SS
        ; line 7: IP FL
        mov     di, si
        add     di, (ROWBYTES * 7) + 4
        call    Print_IP_FL

        pop     es
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret
DrawRegsBox endp

; ------------------------------------------------------------------------------
; Draw the cow below the box.
; Chooses normal or flipped sprite based on CowFlip.
; ------------------------------------------------------------------------------
DrawCow proc
        push    ax
        push    bx
        push    dx
        push    es
        cld

        mov     ax, VSEG
        mov     es, ax

        ; cow starts under the frame
        xor     dx, dx
        mov     dl, [BoxRow]
        add     dx, (BOX_H + COW_GAP)

        ; indent inside saved area
        xor     ax, ax
        mov     al, [BoxCol]
        add     ax, 7

        mov     bl, CLR           ; white foreground

        cmp     byte ptr [CowFlip], 0
        jne     @@flipped

@@normal:
        push    dx
        push    ax
        push    offset cow1
        call    PutStrAtSetFG

        inc     dx
        push    dx
        push    ax
        push    offset cow2
        call    PutStrAtSetFG

        inc     dx
        push    dx
        push    ax
        push    offset cow3
        call    PutStrAtSetFG

        inc     dx
        push    dx
        push    ax
        push    offset cow4
        call    PutStrAtSetFG

        inc     dx
        push    dx
        push    ax
        push    offset cow5
        call    PutStrAtSetFG
        jmp     @@done

@@flipped:
        push    dx
        push    ax
        push    offset cow1f
        call    PutStrAtSetFG

        inc     dx
        push    dx
        push    ax
        push    offset cow2f
        call    PutStrAtSetFG

        inc     dx
        push    dx
        push    ax
        push    offset cow3f
        call    PutStrAtSetFG

        inc     dx
        push    dx
        push    ax
        push    offset cow4f
        call    PutStrAtSetFG

        inc     dx
        push    dx
        push    ax
        push    offset cow5f
        call    PutStrAtSetFG

@@done:
        pop     es
        pop     dx
        pop     bx
        pop     ax
        ret
DrawCow endp

; ------------------------------------------------------------------------------
; Draw a surprise next to the cow.
; ------------------------------------------------------------------------------
DrawPoop proc
        push    ax
        push    bx
        push    cx
        push    dx
        push    di
        push    es
        cld

        mov     ax, VSEG
        mov     es, ax

        ; row near lower part of cow
        xor     dx, dx
        mov     dl, [BoxRow]
        add     dx, POOP_ROW

        ; choose side depending on orientation
        xor     ax, ax
        mov     al, [BoxCol]

        cmp     byte ptr [CowFlip], 0
        jne     @@leftside

        add     ax, SAVE_W
        jmp     @@posok

@@leftside:
        sub     ax, POOP_W

@@posok:
        push    ax
        push    dx
        call    SetPos
        add     sp, 4

        ; top row "  ~~"
        add     di, 4

        mov     al, '~'
        mov     bl, CLR_GREEN
        mov     cx, 2
        call    PutRepCharSetFG

        ; second row "@@@@"
        add     di, ROWBYTES - 8

        mov     al, '@'
        mov     bl, CLR_BROWN
        mov     cx, POOP_W
        call    PutRepCharSetFG

        pop     es
        pop     di
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        ret
DrawPoop endp

; print "AX=hhhh BX=hhhh"
Print_AX_BX proc
        mov     al, 'A'
        PutChar
        mov     al, 'X'
        PutChar
        mov     al, '='
        PutChar
        mov     ax, [RegAX]
        PutHex16
        mov     al, ' '
        PutChar
        mov     al, 'B'
        PutChar
        mov     al, 'X'
        PutChar
        mov     al, '='
        PutChar
        mov     ax, [RegBX]
        PutHex16
        ret
Print_AX_BX endp

; print "CX=hhhh DX=hhhh"
Print_CX_DX proc
        mov     al, 'C'
        PutChar
        mov     al, 'X'
        PutChar
        mov     al, '='
        PutChar
        mov     ax, [RegCX]
        PutHex16
        mov     al, ' '
        PutChar
        mov     al, 'D'
        PutChar
        mov     al, 'X'
        PutChar
        mov     al, '='
        PutChar
        mov     ax, [RegDX]
        PutHex16
        ret
Print_CX_DX endp

; print "SI=hhhh DI=hhhh"
Print_SI_DI proc
        mov     al, 'S'
        PutChar
        mov     al, 'I'
        PutChar
        mov     al, '='
        PutChar
        mov     ax, [RegSI]
        PutHex16
        mov     al, ' '
        PutChar
        mov     al, 'D'
        PutChar
        mov     al, 'I'
        PutChar
        mov     al, '='
        PutChar
        mov     ax, [RegDI]
        PutHex16
        ret
Print_SI_DI endp

; print "BP=hhhh SP=hhhh"
Print_BP_SP proc
        mov     al, 'B'
        PutChar
        mov     al, 'P'
        PutChar
        mov     al, '='
        PutChar
        mov     ax, [RegBP]
        PutHex16
        mov     al, ' '
        PutChar
        mov     al, 'S'
        PutChar
        mov     al, 'P'
        PutChar
        mov     al, '='
        PutChar
        mov     ax, [RegSP]
        PutHex16
        ret
Print_BP_SP endp

; print "CS=hhhh DS=hhhh"
Print_CS_DS proc
        mov     al, 'C'
        PutChar
        mov     al, 'S'
        PutChar
        mov     al, '='
        PutChar
        mov     ax, [RegCS]
        PutHex16
        mov     al, ' '
        PutChar
        mov     al, 'D'
        PutChar
        mov     al, 'S'
        PutChar
        mov     al, '='
        PutChar
        mov     ax, [RegDS]
        PutHex16
        ret
Print_CS_DS endp

; print "ES=hhhh SS=hhhh"
Print_ES_SS proc
        mov     al, 'E'
        PutChar
        mov     al, 'S'
        PutChar
        mov     al, '='
        PutChar
        mov     ax, [RegES]
        PutHex16
        mov     al, ' '
        PutChar
        mov     al, 'S'
        PutChar
        mov     al, 'S'
        PutChar
        mov     al, '='
        PutChar
        mov     ax, [RegSS]
        PutHex16
        ret
Print_ES_SS endp

; print "IP=hhhh FL=hhhh"
Print_IP_FL proc
        mov     al, 'I'
        PutChar
        mov     al, 'P'
        PutChar
        mov     al, '='
        PutChar
        mov     ax, [RegIP]
        PutHex16
        mov     al, ' '
        PutChar
        mov     al, 'F'
        PutChar
        mov     al, 'L'
        PutChar
        mov     al, '='
        PutChar
        mov     ax, [RegFL]
        PutHex16
        ret
Print_IP_FL endp

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

        mov     bx, [bp + 4]      ; row
        mov     ax, bx
        shl     ax, 6             ; row * 64
        mov     di, ax
        mov     ax, bx
        shl     ax, 4             ; row * 16
        add     di, ax            ; row * 80

        mov     ax, [bp + 6]      ; col
        add     di, ax
        shl     di, 1

        pop     bx
        pop     ax
        pop     bp
        ret                       ; caller cleans
SetPos endp

; ------------------------------------------------------------------------------
; pascal
; Put 0-terminated pStr at (row, col), preserving existing attribute byte.
; Entry: stack = row (then) col (then) pStr
; ------------------------------------------------------------------------------
PutStrAt proc
        push    bp
        mov     bp, sp
        push    ax
        push    bx
        push    si
        cld

        mov     si, [bp + 4]      ; pStr
        mov     bx, [bp + 6]      ; col
        mov     ax, [bp + 8]      ; row

        ; call SetPos(row, col) (cdecl)
        push    bx                ; col
        push    ax                ; row
        call    SetPos
        add     sp, 4

@@loop:
        lodsb
        test    al, al
        jz      @@done
        PutCharKeepAttr
        jmp     @@loop

@@done:
        pop     si
        pop     bx
        pop     ax
        pop     bp
        ret     6
PutStrAt endp

; ------------------------------------------------------------------------------
; Put same character CX times while preserving full existing attribute.
; Entry: AL    = character
;        CX    = repeat count
;        ES:DI = screen cell
; Exit:  DI    = updated position
; ------------------------------------------------------------------------------
PutRepCharKeepAttr proc
        jcxz    @@done
@@loop:
        mov     es:[di], al
        add     di, 2
        loop    @@loop
@@done:
        ret
PutRepCharKeepAttr endp

; ------------------------------------------------------------------------------
; Put character while preserving background/blink and forcing foreground.
; Entry: AL    = character
;        BL    = new foreground color (0..0Fh)
;        ES:DI = screen cell
; Exit:  DI    = updated position
; ------------------------------------------------------------------------------
PutCharSetFG proc
        mov     es:[di], al
        mov     ah, es:[di+1]
        and     ah, 0F0h
        or      ah, bl
        mov     es:[di+1], ah
        add     di, 2
        ret
PutCharSetFG endp

; ------------------------------------------------------------------------------
; Put same character CX times while preserving background/blink and forcing fg.
; Entry: AL    = character
;        BL    = new foreground color
;        CX    = repeat count
;        ES:DI = screen cell
; Exit:  DI    = updated position
; ------------------------------------------------------------------------------
PutRepCharSetFG proc
        jcxz    @@done
@@loop:
        call PutCharSetFG
        loop    @@loop
@@done:
        ret
PutRepCharSetFG endp

; ------------------------------------------------------------------------------
; pascal
; Put 0-terminated pStr at (row, col), preserving background/blink and forcing fg.
; Entry: stack = row (then) col (then) pStr
;        BL    = new foreground color
; ------------------------------------------------------------------------------
PutStrAtSetFG proc
        push    bp
        mov     bp, sp
        push    ax
        push    dx
        push    si
        cld

        mov     si, [bp + 4]      ; pStr
        mov     dx, [bp + 6]      ; col
        mov     ax, [bp + 8]      ; row

        ; call SetPos(row, col) (cdecl)
        push    dx                ; col
        push    ax                ; row
        call    SetPos
        add     sp, 4

@@loop:
        lodsb
        test    al, al
        jz      @@done
        call PutCharSetFG
        jmp     @@loop

@@done:
        pop     si
        pop     dx
        pop     ax
        pop     bp
        ret     6
PutStrAtSetFG endp

; ----------------------------- resident data ----------------------------------

.data
SavedSS         dw 0
SavedSP         dw 0

OldOffset       dw 0
OldSegment      dw 0

LastScan        db 0
BoxOn           db 0
BoxRow          db 0
BoxCol          db 0
CowFlip         db 0

RegAX           dw 0
RegBX           dw 0
RegCX           dw 0
RegDX           dw 0
RegSI           dw 0
RegDI           dw 0
RegBP           dw 0
RegSP           dw 0
RegCS           dw 0
RegDS           dw 0
RegES           dw 0
RegSS           dw 0
RegIP           dw 0
RegFL           dw 0

cow1            db '\   ^__^', 0
cow2            db ' \  (oo)\_______', 0
cow3            db '    (__)\  asm  )\/', '\', 0
cow4            db '        ||----w |', 0
cow5            db '        ||     ||', 0

cow1f           db '            ^__^   /', 0
cow2f           db '    _______/(oo)  / ', 0
cow3f           db '/\/(  msa  /(__)    ', 0
cow4f           db '   | w----||        ', 0
cow5f           db '   ||     ||        ', 0

SavedArea       dw (SAVE_W * SAVE_H) dup (0)

EndResident     label byte

end start
