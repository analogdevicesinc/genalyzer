    .text
    .globl bh_apply_asm
    .type  bh_apply_asm, @function
# void bh_apply_asm(const double* sw, const double* I, const double* Q,
#                   double* out, size_t nfft)   // nfft multiple of 4
# rdi=sw  rsi=I  rdx=Q  rcx=out  r8=nfft
bh_apply_asm:
    xor     %rax, %rax                 # rax = sample index i
.Lloop:
    cmp     %r8, %rax
    jae     .Ldone
    vmovupd (%rdi,%rax,8), %ymm0        # sw[i..i+3]
    vmovupd (%rsi,%rax,8), %ymm1        # I[i..i+3]
    vmovupd (%rdx,%rax,8), %ymm2        # Q[i..i+3]
    vmulpd  %ymm1, %ymm0, %ymm1         # ri = sw*I  (r0 r1 r2 r3)
    vmulpd  %ymm2, %ymm0, %ymm2         # rq = sw*Q  (q0 q1 q2 q3)
    vunpcklpd %ymm2, %ymm1, %ymm3       # r0 q0 r2 q2
    vunpckhpd %ymm2, %ymm1, %ymm4       # r1 q1 r3 q3
    vperm2f128 $0x20, %ymm4, %ymm3, %ymm5   # r0 q0 r1 q1
    vperm2f128 $0x31, %ymm4, %ymm3, %ymm6   # r2 q2 r3 q3
    lea     (%rax,%rax,1), %r9          # r9 = 2*i  (out is interleaved)
    vmovupd %ymm5, (%rcx,%r9,8)         # out[2i .. 2i+3]
    vmovupd %ymm6, 32(%rcx,%r9,8)       # out[2i+4 .. 2i+7]
    add     $4, %rax
    jmp     .Lloop
.Ldone:
    vzeroupper
    ret
    .size  bh_apply_asm, .-bh_apply_asm
.section .note.GNU-stack,"",@progbits
