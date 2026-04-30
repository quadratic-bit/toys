.intel_syntax noprefix
.text
.p2align 4
.globl ht_most_frequent_index_ext

# size_t ht_most_frequent_index_ext(const ht_t *ht)
#
# args:
#   rdi = ht
#   rax = best index
#
# Assume:
#   ht != NULL
#   ht->size    != 0
#   ht.entries  = 16
#   ht.size     = 24
#   entry size  = 32
#   entry.count = 8

ht_most_frequent_index_ext:
    mov     rdx, qword ptr [rdi + 16]      # entries
    mov     r10, qword ptr [rdi + 24]      # size

    mov     eax, 1                         # best = 1
    mov     r8,  qword ptr [rdx + 40]      # best_count = entries[1].count

    cmp     r10, 1
    je      .done

    mov     ecx, 2                         # i = 2

.p2align 4
.loop2:
    lea     r11, [rcx + 1]
    cmp     r11, r10
    ja      .tail

    # candidate i
    mov     r9, rcx
    shl     r9, 5                          # i * 32
    mov     rsi, qword ptr [rdx + r9 + 8]  # entries[i].count
    cmp     rsi, r8
    cmova   r8, rsi
    cmova   rax, rcx

    # candidate i+1
    add     r9, 32
    mov     rsi, qword ptr [rdx + r9 + 8]  # entries[i+1].count
    lea     r11, [rcx + 1]
    cmp     rsi, r8
    cmova   r8, rsi
    cmova   rax, r11

    add     rcx, 2
    jmp     .loop2

.tail:
    cmp     rcx, r10
    ja      .done

    mov     r9, rcx
    shl     r9, 5
    mov     rsi, qword ptr [rdx + r9 + 8]
    cmp     rsi, r8
    cmova   r8, rsi
    cmova   rax, rcx

.done:
    ret
