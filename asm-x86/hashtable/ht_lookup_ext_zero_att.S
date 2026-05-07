.intel_syntax noprefix
.text
.p2align 4
.globl ht_lookup

# uint64_t ht_lookup(const ht_t *ht, const void *key, size_t key_len)
#
# Aguments
#   rdi = ht
#   rsi = key
#   rdx = key_len
#
# layout:
#   ht.buckets      +0
#   ht.bucket_count +8
#   ht.entries      +16
#   ht.arena        +40
#
#   entry.prefix8   +0
#   entry.count     +8
#   entry.hash      +16
#   entry.next      +20
#   entry.key_off   +24
#   entry.key_len   +28
#   sizeof(entry)   32

ht_lookup:
    # !ht
    test    rdi, rdi
    jz      .return_zero

    # !key && key_len != 0
    test    rsi, rsi
    jnz     .key_ok
    test    rdx, rdx
    jnz     .return_zero

.key_ok:
    # ht->bucket_count == 0
    mov     rax, qword ptr [rdi + 8]
    test    rax, rax
    jz      .return_zero

    # callee-saved
    push    rbx
    push    rbp
    push    r12
    push    r13
    push    r14
    push    r15
    sub     rsp, 8     # potential memcmp

    mov     r12, rdi   # ht
    mov     r13, rsi   # key
    mov     r14, rdx   # key_len

    # -------- hash32 --------

    mov     r8, r13                     # p
    mov     r9, r14                     # len
    xor     ecx, ecx                    # crc = 0

.p_hash8:
    cmp     r9, 8
    jb      .p_hash4
    crc32   rcx, qword ptr [r8]
    add     r8, 8
    sub     r9, 8
    jmp     .p_hash8

.p_hash4:
    cmp     r9, 4
    jb      .p_hash2
    crc32   ecx, dword ptr [r8]
    add     r8, 4
    sub     r9, 4

.p_hash2:
    cmp     r9, 2
    jb      .p_hash1
    crc32   ecx, word ptr [r8]
    add     r8, 2
    sub     r9, 2

.p_hash1:
    test    r9, r9
    jz      .hash_done
    crc32   ecx, byte ptr [r8]

.hash_done:
    mov     ebp, ecx       # ebp <- hash32

    # ------ prefix8, up to 8 bytes, little-endian -------
    mov     r15, qword ptr [r13]
    cmp     r14, 8
    jae     .prefix_done

    mov     ecx, r14d
    shl     ecx, 3                      # valid bits = key_len * 8
    bzhi    r15, r15, rcx               # prefix8 = low valid bits

.prefix_done:

    # ----- bucket = hash32 & (uint32_t)(bucket_count - 1) -----
    mov     r10, qword ptr [r12 + 8]     # bucket_count
    dec     r10
    and     r10d, ebp                    # bucket

    mov     r11, qword ptr [r12 + 0]     # buckets
    mov     eax, dword ptr [r11 + r10*4] # i = buckets[bucket]
    test    eax, eax
    jz      .not_found

.lookup_loop:
    # e = &entries[i]
    mov     r8, qword ptr [r12 + 16]    # entries
    shl     rax, 5                      # i * 32
    lea     rbx, [r8 + rax]             # e

    # e->hash != hash32 -> skip
    cmp     dword ptr [rbx + 16], ebp
    jne     .next_entry

    # e->key_len != key_len -> ckip
    mov     ecx, dword ptr [rbx + 28]
    cmp     r14, rcx
    jne     .next_entry

    # e->prefix8 != prefix8 -> skip
    cmp     qword ptr [rbx + 0], r15
    jne     .next_entry

    # key_len <= 8 -> verified by prefix
    cmp     r14, 8
    jbe     .found

    # stored = ht->arena + e->key_off
    mov     r8d, dword ptr [rbx + 24]
    add     r8,  qword ptr [r12 + 40]

    # key_len <= 16 -> compare tail 8 bytes
    cmp     r14, 16
    ja      .maybe_17_32

    lea     r10, [r14 - 8]
    mov     rax, qword ptr [r8 + r10]
    xor     rax, qword ptr [r13 + r10]
    jne     .next_entry
    jmp     .found

.maybe_17_32:
    cmp     r14, 32
    ja      .long_memcmp

    # 17..32:
    # compare tail 16 bytes
    #
    # r10 = key_len - 16
    # load covers [key_len - 16, key_len - 1], in bounds for key_len > 16
    lea     r10, [r14 - 16]

    movdqu  xmm0, xmmword ptr [r8  + r10]
    movdqu  xmm1, xmmword ptr [r13 + r10]
    pxor    xmm0, xmm1
    ptest   xmm0, xmm0
    jnz     .next_entry

    # 17..24:
    # prefix8 covers [0..7], tail16 covers rest, with overlap
    cmp     r14, 24
    jbe     .found

    # 25..32:
    # prove middle bytes [8..23]
    # load is in bounds for key_len >= 25.
    movdqu  xmm0, xmmword ptr [r8  + 8]
    movdqu  xmm1, xmmword ptr [r13 + 8]
    pxor    xmm0, xmm1
    ptest   xmm0, xmm0
    jnz     .next_entry

    jmp     .found

.long_memcmp:
    # memcmp(stored, key, key_len)
    mov     rdi, r8
    mov     rsi, r13
    mov     rdx, r14
    call    memcmp@PLT
    test    eax, eax
    jne     .next_entry

.found:
    # return e->count
    mov     rax, qword ptr [rbx + 8]
    jmp     .done

.next_entry:
    mov     eax, dword ptr [rbx + 20]   # i = e->next
    test    eax, eax
    jne     .lookup_loop

.not_found:
    xor     eax, eax

.done:
    add     rsp, 8
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbp
    pop     rbx
    ret

.return_zero:
    xor     eax, eax
    ret
