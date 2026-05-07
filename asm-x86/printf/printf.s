.intel_syntax noprefix

.extern __real_printf
.global __wrap_printf

.section .rodata
hex_digits:
        .ascii "0123456789abcdef"
null_string:
        .asciz "(null)"
count_dispatch_jt:
        .long .count_one - count_dispatch_jt      # 'b'
        .long .count_one - count_dispatch_jt      # 'c'
        .long .count_one - count_dispatch_jt      # 'd'

        .rept 'n' - 'e' + 1
                .long .count_loop - count_dispatch_jt
        .endr

        .long .count_one - count_dispatch_jt      # 'o'

        .rept 'r' - 'p' + 1
                .long .count_loop - count_dispatch_jt
        .endr

        .long .count_one - count_dispatch_jt      # 's'

        .rept 'w' - 't' + 1
                .long .count_loop - count_dispatch_jt
        .endr

        .long .count_one - count_dispatch_jt      # 'x'

# ==============================================================================
# printf
# SysV entry
# Arguments in rdi (C-string) then rsi, rdx, rcx, r8, r9 (vargs); other in stack
# Preserve: rbx, rsp, rbp, r12, r13, r14, r15
# Destroy : rax, arguments, r10, r11
# ==============================================================================
.section .text
__wrap_printf:
        mov r10, rsp         # original stack

        push rbx
        push rbp
        push r12
        push r13
        push r14
        push r15

        sub rsp, 48
        mov rbp, rsp

        #   [rbp + 0],         cleanup qword count
        mov [rbp + 8],  rsi  # 1st argument
        mov [rbp + 16], rdx  # 2nd argument
        mov [rbp + 24], rcx  # 3rd argument
        mov [rbp + 32], r8   # 4th argument
        mov [rbp + 40], r9   # 5th argument

        mov rbx, rdi         # saved fmt
        mov r12, r10         # preserve original caller stack across C call

        xor eax, eax         # count of consumed conversions
        mov r11, rbx         # scan pointer

.count_loop:
        mov dl, byte ptr [r11]
        test dl, dl
        je .count_done
        inc r11
        cmp dl, '%'
        jne .count_loop

        mov dl, byte ptr [r11]
        test dl, dl
        je .count_done
        inc r11
        cmp dl, '%'
        je .count_loop

        movzx edx, dl
        sub edx, 'b'
        cmp edx, 'x' - 'b'
        ja .count_loop

        lea r10, [rip + count_dispatch_jt]
        movsxd r9, dword ptr [r10 + rdx*4]
        add r10, r9
        jmp r10

.count_one:
        inc eax
        jmp .count_loop

.count_done:
        mov ecx, eax                    # number of consumed varargs
        mov r14d, ecx                   # save original consumed vararg count
        lea edx, [rcx + 1]              # total qwords: fmt + args
        mov qword ptr [rbp + 0], rdx
        test edx, 1
        jnz .no_pad
        push 0                          # stack alignment my princess
        inc qword ptr [rbp + 0]

.no_pad:
        test ecx, ecx
        jz .push_fmt

.push_args:
        cmp ecx, 5
        ja .push_stack_arg

        mov eax, ecx
        shl eax, 3                       # ecx * 8
        cdqe                             # zero extend eax to rax
        push qword ptr [rbp + rax]       # [rbp+8], [rbp+16], ... [rbp+40]
        dec ecx
        jnz .push_args
        jmp .push_fmt

.push_stack_arg:
        mov eax, ecx
        sub eax, 5
        cdqe                             # zero extend eax to rax
        push qword ptr [r12 + rax*8]
        dec ecx
        jnz .push_args

.push_fmt:
        push rbx
        call printf_cdecl

        mov r13d, eax
        mov rdx, qword ptr [rbp]
        shl rdx, 3
        add rsp, rdx

        xor r15d, r15d                 # qwords pushed
        mov edx, r14d                  # original consumed vararg count

.real_push_extra:
        cmp edx, 5
        jle .real_align
        mov eax, edx
        sub eax, 5
        cdqe
        push qword ptr [r12 + rax*8]   # original caller stack args
        inc r15d
        dec edx
        jmp .real_push_extra

.real_align:
        test r15d, 1
        jnz .real_call
        push 0                         # my baby SysV call princess alignment
        inc r15d

.real_call:
        mov rdi, rbx
        mov rsi, qword ptr [rbp + 8]
        mov rdx, qword ptr [rbp + 16]
        mov rcx, qword ptr [rbp + 24]
        mov r8,  qword ptr [rbp + 32]
        mov r9,  qword ptr [rbp + 40]
        xor eax, eax                   # vector args soon :copium:
        call __real_printf

        mov edx, r15d
        shl rdx, 3
        add rsp, rdx

        mov eax, r13d                  # return OUR formatter's count

        add rsp, 48
        pop r15
        pop r14
        pop r13
        pop r12
        pop rbp
        pop rbx
        mov eax, r11d
        ret


# ==============================================================================
# printf_cdecl
# printf implementation in cdecl
# Expects on the stack:
# - return address
# - format string
# - arguments
# supported: %x %o %b %d %c %s %%
# ==============================================================================
printf_cdecl:
        push rbp
        mov rbp, rsp

        push rbx
        push r12
        push r13
        push r14
        sub rsp, 112                    # 64-byte output buffer + 48-byte area

        # Stack layout:
        # higher addresses
        #     ...
        # [rbp + 40]  arg3
        # [rbp + 32]  arg2
        # [rbp + 24]  arg1
        # [rbp + 16]  fmt
        # [rbp +  8]  return address
        # [rbp +  0]  saved caller rbp
        # [rbp -  8]  saved rbx
        # [rbp - 16]  saved r12
        # [rbp - 24]  saved r13
        # [rbp - 32]  saved r14
        # [rbp - 33]  \
        #    ...       > 112 bytes of local space
        # [rbp -144] /
        # lower addresses

        xor r12d, r12d                  # total characters written
        xor r13d, r13d                  # buffered byte count
        mov rbx, qword ptr [rbp + 16]   # fmt
        lea r10, [rbp + 24]             # first variadic argument

.parse_loop:
        mov al, byte ptr [rbx]
        test al, al
        je .flush_and_done
        inc rbx
        cmp al, '%'
        jne .buffer_char_al

        mov dl, byte ptr [rbx]
        test dl, dl
        je .buffer_percent_and_done
        inc rbx

        cmp dl, '%'
        je .buffer_char_dl
        cmp dl, 'x'
        je .emit_hex
        cmp dl, 'o'
        je .emit_octal
        cmp dl, 'b'
        je .emit_binary
        cmp dl, 'd'
        je .emit_decimal
        cmp dl, 'c'
        je .emit_char_arg
        cmp dl, 's'
        je .emit_string

        cmp r13d, 62
        ja .flush_then_buffer_unknown
        lea r11, [rbp - 112]            # rbp - 112 = array base pointer
        mov byte ptr [r11 + r13], '%'
        mov byte ptr [r11 + r13 + 1], dl
        add r13d, 2
        jmp .parse_loop

.flush_then_buffer_unknown:
        call .flush_buf
        lea r11, [rbp - 112]
        mov byte ptr [r11], '%'
        mov byte ptr [r11 + 1], dl
        mov r13d, 2
        jmp .parse_loop

.buffer_char_al:
        cmp r13d, 64
        jne .store_char_al
        call .flush_buf
.store_char_al:
        lea r11, [rbp - 112]
        mov byte ptr [r11 + r13], al
        inc r13d
        jmp .parse_loop

.buffer_char_dl:
        cmp r13d, 64
        jne .store_char_dl
        call .flush_buf
.store_char_dl:
        lea r11, [rbp - 112]
        mov byte ptr [r11 + r13], dl
        inc r13d
        jmp .parse_loop

# ===================================== %% =====================================

.buffer_percent_and_done:
        cmp r13d, 64
        jne .store_percent_and_done
        call .flush_buf
.store_percent_and_done:
        lea r11, [rbp - 112]
        mov byte ptr [r11 + r13], '%'
        inc r13d
        jmp .flush_and_done

# ===================================== %c =====================================

.emit_char_arg:
        mov eax, dword ptr [r10]
        add r10, 8
        cmp r13d, 64
        jne .store_char_arg
        call .flush_buf
.store_char_arg:
        lea r11, [rbp - 112]
        mov byte ptr [r11 + r13], al
        inc r13d
        jmp .parse_loop

# ===================================== %s =====================================

.emit_string:
        mov r14, qword ptr [r10]
        add r10, 8

        test r14, r14
        jne .string_loop
        lea r14, [rip + null_string]

.string_loop:
        cmp r13d, 64
        jne .store_string_char
        call .flush_buf

.store_string_char:
        mov al, byte ptr [r14]
        test al, al
        je .parse_loop
        lea r11, [rbp - 112]
        mov byte ptr [r11 + r13], al
        inc r13d
        inc r14
        jmp .string_loop

# =============================== %x, %o, %b ===================================

.emit_hex:
        mov eax, dword ptr [r10]
        add r10, 8
        mov ecx, 4
        jmp .emit_unsigned_base

.emit_octal:
        mov eax, dword ptr [r10]
        add r10, 8
        mov ecx, 3
        jmp .emit_unsigned_base

.emit_binary:
        mov eax, dword ptr [r10]
        add r10, 8
        mov ecx, 1
        jmp .emit_unsigned_base

# base stored at ecx, in number of bits
.emit_unsigned_base:
        test eax, eax
        jne .unsigned_convert

        cmp r13d, 64
        jne .store_zero_unsigned
        call .flush_buf
.store_zero_unsigned:
        lea r11, [rbp - 112]
        mov byte ptr [r11 + r13], '0'
        inc r13d
        jmp .parse_loop

.unsigned_convert:
        lea rsi, [rbp - 16]             # temp conversion end
        xor r14d, r14d                  # digit count

        cmp ecx, 4
        je .hex_loop
        cmp ecx, 3
        je .oct_loop
        jmp .bin_loop

.hex_loop:
        mov edx, eax
        and edx, 0x0f
        lea r11, [rip + hex_digits]
        mov dl, byte ptr [r11 + rdx]
        dec rsi
        mov byte ptr [rsi], dl
        shr eax, 4
        inc r14d
        test eax, eax
        jne .hex_loop
        jmp .copy_digits

.oct_loop:
        mov edx, eax
        and edx, 7
        add dl, '0'
        dec rsi
        mov byte ptr [rsi], dl
        shr eax, 3
        inc r14d
        test eax, eax
        jne .oct_loop
        jmp .copy_digits

.bin_loop:
        mov edx, eax
        and edx, 1
        add dl, '0'
        dec rsi
        mov byte ptr [rsi], dl
        shr eax, 1
        inc r14d
        test eax, eax
        jne .bin_loop

.copy_digits:
        mov eax, 64
        sub eax, r13d
        cmp r14d, eax
        jbe .copy_digits_no_flush
        call .flush_buf

.copy_digits_no_flush:
        lea r11, [rbp - 112]
        add r11, r13
        mov ecx, r14d
.copy_digits_loop:
        mov al, byte ptr [rsi]
        mov byte ptr [r11], al
        inc rsi
        inc r11
        dec ecx
        jnz .copy_digits_loop
        add r13d, r14d
        jmp .parse_loop

# ===================================== %d =====================================

.emit_decimal:
        mov eax, dword ptr [r10]
        add r10, 8

        test eax, eax
        jns .decimal_positive

        cmp r13d, 64
        jne .store_minus
        call .flush_buf
.store_minus:
        lea r11, [rbp - 112]
        mov byte ptr [r11 + r13], '-'
        inc r13d
        neg eax

.decimal_positive:
        test eax, eax
        jne .decimal_convert

        cmp r13d, 64
        jne .store_zero_decimal
        call .flush_buf
.store_zero_decimal:
        lea r11, [rbp - 112]
        mov byte ptr [r11 + r13], '0'
        inc r13d
        jmp .parse_loop

.decimal_convert:
        lea rsi, [rbp - 16]             # temp conversion end
        xor r14d, r14d                  # digit count

.decimal_loop:
        xor edx, edx
        mov ecx, 10
        div ecx                         # unsigned divide edx:eax by 10
                                        # eax = quotient, edx = remainder
        add dl, '0'
        dec rsi
        mov byte ptr [rsi], dl
        inc r14d
        test eax, eax
        jne .decimal_loop
        jmp .copy_digits

# ================================ Buffer handling =============================

.flush_and_done:
        call .flush_buf
        mov eax, r12d
        add rsp, 112
        pop r14
        pop r13
        pop r12
        pop rbx
        pop rbp
        ret

.flush_buf:
        test r13d, r13d
        jz .flush_ret

        push rax
        push rdx
        push rsi

        mov eax, 1 # write
        mov edi, 1 # fd=1 stdout
        lea rsi, [rbp - 112] # const char *buf
        mov edx, r13d # size_t count
        syscall

        pop rsi
        pop rdx
        pop rax

        add r12d, r13d
        xor r13d, r13d
.flush_ret:
        ret
