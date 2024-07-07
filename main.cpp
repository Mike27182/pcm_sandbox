#include "pmc.h"
#include <array>
#include <sys/mman.h>
#include <cstdio>
#include <immintrin.h>
#include <iostream>

using namespace std;

const uint64_t bufSize = 1024*64;

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test1x1( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"

        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        "2:\n\t"
        "vpaddd                   (%[p]), %%ymm0, %%ymm0\n\t"
        "add $32, %[p]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        "jne 2b\n\t"

        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [bufSize] "i" (bufSize)
        : "ymm0", "xmm1"
    );

    return sum;
}

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test1x2( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"

        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        "2:\n\t"
        "vpaddd                   (%[p]), %%ymm0, %%ymm0\n\t"
        "vpaddd                 32(%[p]), %%ymm1, %%ymm1\n\t"
        "add $64, %[p]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        "jne 2b\n\t"

        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"

        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [bufSize] "i" (bufSize)
        : "ymm0", "ymm1", "xmm0", "xmm1"
    );

    return sum;
}

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test4x4( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n\t"
        "vpxor %%ymm2, %%ymm2, %%ymm2\n\t"
        "vpxor %%ymm3, %%ymm3, %%ymm3\n\t"
        "vpxor %%ymm4, %%ymm4, %%ymm4\n\t"
        "vpxor %%ymm5, %%ymm5, %%ymm5\n\t"
        "vpxor %%ymm6, %%ymm6, %%ymm6\n\t"
        "vpxor %%ymm7, %%ymm7, %%ymm7\n\t"
        "vpxor %%ymm8, %%ymm8, %%ymm8\n\t"
        "vpxor %%ymm9, %%ymm9, %%ymm9\n\t"
        "vpxor %%ymm10, %%ymm10, %%ymm10\n\t"
        "vpxor %%ymm11, %%ymm11, %%ymm11\n\t"
        "vpxor %%ymm12, %%ymm12, %%ymm12\n\t"
        "vpxor %%ymm13, %%ymm13, %%ymm13\n\t"
        "vpxor %%ymm14, %%ymm14, %%ymm14\n\t"
        "vpxor %%ymm15, %%ymm15, %%ymm15\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"

        ".align 8\n\t"
        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        ".align 8\n\t"
        "2:\n\t"
        "vpaddd                     (%[p]), %%ymm0, %%ymm0\n\t"
        "vpaddd                   32(%[p]), %%ymm1, %%ymm1\n\t"
        "vpaddd                 + 64(%[p]), %%ymm2, %%ymm2\n\t"
        "vpaddd                 + 96(%[p]), %%ymm3, %%ymm3\n\t"
        "vpaddd 1*%c[blockSize]     (%[p]), %%ymm4, %%ymm4\n\t"
        "vpaddd 1*%c[blockSize] + 32(%[p]), %%ymm5, %%ymm5\n\t"
        "vpaddd 1*%c[blockSize] + 64(%[p]), %%ymm6, %%ymm6\n\t"
        "vpaddd 1*%c[blockSize] + 96(%[p]), %%ymm7, %%ymm7\n\t"
        "vpaddd 2*%c[blockSize]     (%[p]), %%ymm8, %%ymm8\n\t"
        "vpaddd 2*%c[blockSize] + 32(%[p]), %%ymm9, %%ymm9\n\t"
        "vpaddd 2*%c[blockSize] + 64(%[p]), %%ymm10, %%ymm10\n\t"
        "vpaddd 2*%c[blockSize] + 96(%[p]), %%ymm11, %%ymm11\n\t"
        "vpaddd 3*%c[blockSize]     (%[p]), %%ymm12, %%ymm12\n\t"
        "vpaddd 3*%c[blockSize] + 32(%[p]), %%ymm13, %%ymm13\n\t"
        "vpaddd 3*%c[blockSize] + 64(%[p]), %%ymm14, %%ymm14\n\t"
        "vpaddd 3*%c[blockSize] + 96(%[p]), %%ymm15, %%ymm15\n\t"
        "add $128, %[p]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        ".align 8\n\t"
        "jne 2b\n\t"

        "add $%c[blockSize3], %[p]\n\t"
        ".align 8\n\t"
        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm3, %%ymm2, %%ymm2\n\t"
        "vpaddd %%ymm5, %%ymm4, %%ymm4\n\t"
        "vpaddd %%ymm7, %%ymm6, %%ymm6\n\t"
        "vpaddd %%ymm9, %%ymm8, %%ymm8\n\t"
        "vpaddd %%ymm11, %%ymm10, %%ymm10\n\t"
        "vpaddd %%ymm13, %%ymm12, %%ymm12\n\t"
        "vpaddd %%ymm15, %%ymm14, %%ymm14\n\t"

        "vpaddd %%ymm2, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm6, %%ymm4, %%ymm4\n\t"
        "vpaddd %%ymm10, %%ymm8, %%ymm8\n\t"
        "vpaddd %%ymm14, %%ymm12, %%ymm12\n\t"

        "vpaddd %%ymm4, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm12, %%ymm8, %%ymm8\n\t"

        "vpaddd %%ymm8, %%ymm0, %%ymm0\n\t"

        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [blockSize3] "i" (blockSize*3), [bufSize] "i" (bufSize)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15", "xmm0", "xmm1"
    );

    return sum;
}

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test2x1_mov( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"

        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        "2:\n\t"
        "vmovdqa                (%[p]), %%ymm2\n\t"
        "vmovdqa 1*%c[blockSize](%[p]), %%ymm3\n\t"
        "vpaddd %%ymm2, %%ymm3, %%ymm4\n\t"
        "vpaddd %%ymm4, %%ymm0, %%ymm0\n\t"
        "add $32, %[p]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        "jne 2b\n\t"

        "add $%c[blockSize], %[p]\n\t"
        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"

        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [blockSize3] "i" (blockSize*3), [bufSize] "i" (bufSize)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "xmm0", "xmm1"
    );

    return sum;
}

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test4x1_mov( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n\t"
        "vpxor %%ymm2, %%ymm2, %%ymm2\n\t"
        "vpxor %%ymm3, %%ymm3, %%ymm3\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"

        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        "2:\n\t"
        "vmovdqa                (%[p]), %%ymm4\n\t"
        "vmovdqa 1*%c[blockSize](%[p]), %%ymm5\n\t"
        "vmovdqa 2*%c[blockSize](%[p]), %%ymm6\n\t"
        "vmovdqa 3*%c[blockSize](%[p]), %%ymm7\n\t"
        //"vpaddd %%ymm4, %%ymm0, %%ymm0\n\t"
        //"vpaddd %%ymm5, %%ymm1, %%ymm1\n\t"
        //"vpaddd %%ymm6, %%ymm2, %%ymm2\n\t"
        //"vpaddd %%ymm7, %%ymm3, %%ymm3\n\t"
        "add $32, %[p]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        "jne 2b\n\t"

        "add $%c[blockSize3], %[p]\n\t"
        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm3, %%ymm2, %%ymm2\n\t"

        "vpaddd %%ymm2, %%ymm0, %%ymm0\n\t"

        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [blockSize3] "i" (blockSize*3), [bufSize] "i" (bufSize)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "xmm0", "xmm1"
    );

    return sum;
}

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test4x1( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n\t"
        "vpxor %%ymm2, %%ymm2, %%ymm2\n\t"
        "vpxor %%ymm3, %%ymm3, %%ymm3\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"

        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        "2:\n\t"
        "vpaddd                     (%[p]), %%ymm0, %%ymm0\n\t"
        "vpaddd 1*%c[blockSize]     (%[p]), %%ymm1, %%ymm1\n\t"
        "vpaddd 2*%c[blockSize]     (%[p]), %%ymm2, %%ymm2\n\t"
        "vpaddd 3*%c[blockSize]     (%[p]), %%ymm3, %%ymm3\n\t"
        "add $32, %[p]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        "jne 2b\n\t"

        "add $%c[blockSize3], %[p]\n\t"
        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm3, %%ymm2, %%ymm2\n\t"

        "vpaddd %%ymm2, %%ymm0, %%ymm0\n\t"

        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [blockSize3] "i" (blockSize*3), [bufSize] "i" (bufSize)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "xmm0", "xmm1"
    );

    return sum;
}

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test4x2( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n\t"
        "vpxor %%ymm2, %%ymm2, %%ymm2\n\t"
        "vpxor %%ymm3, %%ymm3, %%ymm3\n\t"
        "vpxor %%ymm4, %%ymm4, %%ymm4\n\t"
        "vpxor %%ymm5, %%ymm5, %%ymm5\n\t"
        "vpxor %%ymm6, %%ymm6, %%ymm6\n\t"
        "vpxor %%ymm7, %%ymm7, %%ymm7\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"

        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        "2:\n\t"
        "vpaddd                     (%[p]), %%ymm0, %%ymm0\n\t"
        "vpaddd                   32(%[p]), %%ymm1, %%ymm1\n\t"
        "vpaddd 1*%c[blockSize]     (%[p]), %%ymm2, %%ymm2\n\t"
        "vpaddd 1*%c[blockSize] + 32(%[p]), %%ymm3, %%ymm3\n\t"
        "vpaddd 2*%c[blockSize]     (%[p]), %%ymm4, %%ymm4\n\t"
        "vpaddd 2*%c[blockSize] + 32(%[p]), %%ymm5, %%ymm5\n\t"
        "vpaddd 3*%c[blockSize]     (%[p]), %%ymm6, %%ymm6\n\t"
        "vpaddd 3*%c[blockSize] + 32(%[p]), %%ymm7, %%ymm7\n\t"
        "add $64, %[p]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        "jne 2b\n\t"

        "add $%c[blockSize3], %[p]\n\t"
        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm3, %%ymm2, %%ymm2\n\t"
        "vpaddd %%ymm5, %%ymm4, %%ymm4\n\t"
        "vpaddd %%ymm7, %%ymm6, %%ymm6\n\t"

        "vpaddd %%ymm2, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm6, %%ymm4, %%ymm4\n\t"

        "vpaddd %%ymm4, %%ymm0, %%ymm0\n\t"

        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [blockSize3] "i" (blockSize*3), [bufSize] "i" (bufSize)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "xmm0", "xmm1"
    );

    return sum;
}

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test8x2( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n\t"
        "vpxor %%ymm2, %%ymm2, %%ymm2\n\t"
        "vpxor %%ymm3, %%ymm3, %%ymm3\n\t"
        "vpxor %%ymm4, %%ymm4, %%ymm4\n\t"
        "vpxor %%ymm5, %%ymm5, %%ymm5\n\t"
        "vpxor %%ymm6, %%ymm6, %%ymm6\n\t"
        "vpxor %%ymm7, %%ymm7, %%ymm7\n\t"
        "vpxor %%ymm8, %%ymm8, %%ymm8\n\t"
        "vpxor %%ymm9, %%ymm9, %%ymm9\n\t"
        "vpxor %%ymm10, %%ymm10, %%ymm10\n\t"
        "vpxor %%ymm11, %%ymm11, %%ymm11\n\t"
        "vpxor %%ymm12, %%ymm12, %%ymm12\n\t"
        "vpxor %%ymm13, %%ymm13, %%ymm13\n\t"
        "vpxor %%ymm14, %%ymm14, %%ymm14\n\t"
        "vpxor %%ymm15, %%ymm15, %%ymm15\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"

        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        "2:\n\t"
        "vpaddd                      (%[p]), %%ymm0, %%ymm0\n\t"
        "vpaddd                 + 32 (%[p]), %%ymm1, %%ymm1\n\t"
        "vpaddd 1*%c[blockSize]      (%[p]), %%ymm2, %%ymm2\n\t"
        "vpaddd 1*%c[blockSize] + 32 (%[p]), %%ymm3, %%ymm3\n\t"
        "vpaddd 2*%c[blockSize]      (%[p]), %%ymm4, %%ymm4\n\t"
        "vpaddd 2*%c[blockSize] + 32 (%[p]), %%ymm5, %%ymm5\n\t"
        "vpaddd 3*%c[blockSize]      (%[p]), %%ymm6, %%ymm6\n\t"
        "vpaddd 3*%c[blockSize] + 32 (%[p]), %%ymm7, %%ymm7\n\t"
        "vpaddd 4*%c[blockSize]      (%[p]), %%ymm8, %%ymm8\n\t"
        "vpaddd 4*%c[blockSize] + 32 (%[p]), %%ymm9, %%ymm9\n\t"
        "vpaddd 5*%c[blockSize]      (%[p]), %%ymm10, %%ymm10\n\t"
        "vpaddd 5*%c[blockSize] + 32 (%[p]), %%ymm11, %%ymm11\n\t"
        "vpaddd 6*%c[blockSize]      (%[p]), %%ymm12, %%ymm12\n\t"
        "vpaddd 6*%c[blockSize] + 32 (%[p]), %%ymm13, %%ymm13\n\t"
        "vpaddd 7*%c[blockSize]      (%[p]), %%ymm14, %%ymm14\n\t"
        "vpaddd 7*%c[blockSize] + 32 (%[p]), %%ymm15, %%ymm15\n\t"
        "add $64, %[p]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        "jne 2b\n\t"

        "add $%c[blockSize7], %[p]\n\t"
        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm3, %%ymm2, %%ymm2\n\t"
        "vpaddd %%ymm5, %%ymm4, %%ymm4\n\t"
        "vpaddd %%ymm7, %%ymm6, %%ymm6\n\t"
        "vpaddd %%ymm9, %%ymm8, %%ymm8\n\t"
        "vpaddd %%ymm11, %%ymm10, %%ymm10\n\t"
        "vpaddd %%ymm13, %%ymm12, %%ymm12\n\t"
        "vpaddd %%ymm15, %%ymm14, %%ymm14\n\t"

        "vpaddd %%ymm2, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm6, %%ymm4, %%ymm4\n\t"
        "vpaddd %%ymm10, %%ymm8, %%ymm8\n\t"
        "vpaddd %%ymm14, %%ymm12, %%ymm12\n\t"

        "vpaddd %%ymm4, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm12, %%ymm8, %%ymm8\n\t"

        "vpaddd %%ymm8, %%ymm0, %%ymm0\n\t"

        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [blockSize7] "i" (blockSize*7), [bufSize] "i" (bufSize)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15", "xmm0", "xmm1"
    );

    return sum;
}

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test16x1( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n\t"
        "vpxor %%ymm2, %%ymm2, %%ymm2\n\t"
        "vpxor %%ymm3, %%ymm3, %%ymm3\n\t"
        "vpxor %%ymm4, %%ymm4, %%ymm4\n\t"
        "vpxor %%ymm5, %%ymm5, %%ymm5\n\t"
        "vpxor %%ymm6, %%ymm6, %%ymm6\n\t"
        "vpxor %%ymm7, %%ymm7, %%ymm7\n\t"
        "vpxor %%ymm8, %%ymm8, %%ymm8\n\t"
        "vpxor %%ymm9, %%ymm9, %%ymm9\n\t"
        "vpxor %%ymm10, %%ymm10, %%ymm10\n\t"
        "vpxor %%ymm11, %%ymm11, %%ymm11\n\t"
        "vpxor %%ymm12, %%ymm12, %%ymm12\n\t"
        "vpxor %%ymm13, %%ymm13, %%ymm13\n\t"
        "vpxor %%ymm14, %%ymm14, %%ymm14\n\t"
        "vpxor %%ymm15, %%ymm15, %%ymm15\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"

        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        "2:\n\t"
        "vpaddd                  (%[p]), %%ymm0, %%ymm0\n\t"
        "vpaddd  1*%c[blockSize] (%[p]), %%ymm1, %%ymm1\n\t"
        "vpaddd  2*%c[blockSize] (%[p]), %%ymm2, %%ymm2\n\t"
        "vpaddd  3*%c[blockSize] (%[p]), %%ymm3, %%ymm3\n\t"
        "vpaddd  4*%c[blockSize] (%[p]), %%ymm4, %%ymm4\n\t"
        "vpaddd  5*%c[blockSize] (%[p]), %%ymm5, %%ymm5\n\t"
        "vpaddd  6*%c[blockSize] (%[p]), %%ymm6, %%ymm6\n\t"
        "vpaddd  7*%c[blockSize] (%[p]), %%ymm7, %%ymm7\n\t"
        "vpaddd  8*%c[blockSize] (%[p]), %%ymm8, %%ymm8\n\t"
        "vpaddd  9*%c[blockSize] (%[p]), %%ymm9, %%ymm9\n\t"
        "vpaddd 10*%c[blockSize] (%[p]), %%ymm10, %%ymm10\n\t"
        "vpaddd 11*%c[blockSize] (%[p]), %%ymm11, %%ymm11\n\t"
        "vpaddd 12*%c[blockSize] (%[p]), %%ymm12, %%ymm12\n\t"
        "vpaddd 13*%c[blockSize] (%[p]), %%ymm13, %%ymm13\n\t"
        "vpaddd 14*%c[blockSize] (%[p]), %%ymm14, %%ymm14\n\t"
        "vpaddd 15*%c[blockSize] (%[p]), %%ymm15, %%ymm15\n\t"
        "add $32, %[p]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        "jne 2b\n\t"

        "add $%c[blockSize15], %[p]\n\t"
        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm3, %%ymm2, %%ymm2\n\t"
        "vpaddd %%ymm5, %%ymm4, %%ymm4\n\t"
        "vpaddd %%ymm7, %%ymm6, %%ymm6\n\t"
        "vpaddd %%ymm9, %%ymm8, %%ymm8\n\t"
        "vpaddd %%ymm11, %%ymm10, %%ymm10\n\t"
        "vpaddd %%ymm13, %%ymm12, %%ymm12\n\t"
        "vpaddd %%ymm15, %%ymm14, %%ymm14\n\t"

        "vpaddd %%ymm2, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm6, %%ymm4, %%ymm4\n\t"
        "vpaddd %%ymm10, %%ymm8, %%ymm8\n\t"
        "vpaddd %%ymm14, %%ymm12, %%ymm12\n\t"

        "vpaddd %%ymm4, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm12, %%ymm8, %%ymm8\n\t"

        "vpaddd %%ymm8, %%ymm0, %%ymm0\n\t"

        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [blockSize15] "i" (blockSize*15), [bufSize] "i" (bufSize)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15", "xmm0", "xmm1"
    );

    return sum;
}

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test16x1_mov( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n\t"
        "vpxor %%ymm2, %%ymm2, %%ymm2\n\t"
        "vpxor %%ymm3, %%ymm3, %%ymm3\n\t"
        "vpxor %%ymm4, %%ymm4, %%ymm4\n\t"
        "vpxor %%ymm5, %%ymm5, %%ymm5\n\t"
        "vpxor %%ymm6, %%ymm6, %%ymm6\n\t"
        "vpxor %%ymm7, %%ymm7, %%ymm7\n\t"
        "vpxor %%ymm8, %%ymm8, %%ymm8\n\t"
        "vpxor %%ymm9, %%ymm9, %%ymm9\n\t"
        "vpxor %%ymm10, %%ymm10, %%ymm10\n\t"
        "vpxor %%ymm11, %%ymm11, %%ymm11\n\t"
        "vpxor %%ymm12, %%ymm12, %%ymm12\n\t"
        "vpxor %%ymm13, %%ymm13, %%ymm13\n\t"
        "vpxor %%ymm14, %%ymm14, %%ymm14\n\t"
        "vpxor %%ymm15, %%ymm15, %%ymm15\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"

        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        "2:\n\t"
        "vmovdqa                     (%[p]), %%ymm0\n\t"
        "vmovdqa     1*%c[blockSize] (%[p]), %%ymm1\n\t"
        "vmovdqa     2*%c[blockSize] (%[p]), %%ymm2\n\t"
        "vmovdqa     3*%c[blockSize] (%[p]), %%ymm3\n\t"
        "vmovdqa     4*%c[blockSize] (%[p]), %%ymm4\n\t"
        "vmovdqa     5*%c[blockSize] (%[p]), %%ymm5\n\t"
        "vmovdqa     6*%c[blockSize] (%[p]), %%ymm6\n\t"
        "vmovdqa     7*%c[blockSize] (%[p]), %%ymm7\n\t"
        "vmovdqa     8*%c[blockSize] (%[p]), %%ymm8\n\t"
        "vmovdqa     9*%c[blockSize] (%[p]), %%ymm9\n\t"
        "vmovdqa    10*%c[blockSize] (%[p]), %%ymm10\n\t"
        "vmovdqa    11*%c[blockSize] (%[p]), %%ymm11\n\t"
        "vmovdqa    12*%c[blockSize] (%[p]), %%ymm12\n\t"
        "vmovdqa    13*%c[blockSize] (%[p]), %%ymm13\n\t"
        "vmovdqa    14*%c[blockSize] (%[p]), %%ymm14\n\t"
        "vmovdqa    15*%c[blockSize] (%[p]), %%ymm15\n\t"
        "add $32, %[p]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        "jne 2b\n\t"

        "add $%c[blockSize15], %[p]\n\t"
        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm3, %%ymm2, %%ymm2\n\t"
        "vpaddd %%ymm5, %%ymm4, %%ymm4\n\t"
        "vpaddd %%ymm7, %%ymm6, %%ymm6\n\t"
        "vpaddd %%ymm9, %%ymm8, %%ymm8\n\t"
        "vpaddd %%ymm11, %%ymm10, %%ymm10\n\t"
        "vpaddd %%ymm13, %%ymm12, %%ymm12\n\t"
        "vpaddd %%ymm15, %%ymm14, %%ymm14\n\t"

        "vpaddd %%ymm2, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm6, %%ymm4, %%ymm4\n\t"
        "vpaddd %%ymm10, %%ymm8, %%ymm8\n\t"
        "vpaddd %%ymm14, %%ymm12, %%ymm12\n\t"

        "vpaddd %%ymm4, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm12, %%ymm8, %%ymm8\n\t"

        "vpaddd %%ymm8, %%ymm0, %%ymm0\n\t"

        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [blockSize15] "i" (blockSize*15), [bufSize] "i" (bufSize)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15", "xmm0", "xmm1"
    );

    return sum;
}

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test8x1_mov( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n\t"
        "vpxor %%ymm2, %%ymm2, %%ymm2\n\t"
        "vpxor %%ymm3, %%ymm3, %%ymm3\n\t"
        "vpxor %%ymm4, %%ymm4, %%ymm4\n\t"
        "vpxor %%ymm5, %%ymm5, %%ymm5\n\t"
        "vpxor %%ymm6, %%ymm6, %%ymm6\n\t"
        "vpxor %%ymm7, %%ymm7, %%ymm7\n\t"
        "vpxor %%ymm8, %%ymm8, %%ymm8\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"

        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        "2:\n\t"
        "vmovdqa                     (%[p]), %%ymm0\n\t"
        "vmovdqa 1*%c[blockSize]     (%[p]), %%ymm1\n\t"
        "vmovdqa 2*%c[blockSize]     (%[p]), %%ymm2\n\t"
        "vmovdqa 3*%c[blockSize]     (%[p]), %%ymm3\n\t"
        "vmovdqa 4*%c[blockSize]     (%[p]), %%ymm4\n\t"
        "vmovdqa 5*%c[blockSize]     (%[p]), %%ymm5\n\t"
        "vmovdqa 6*%c[blockSize]     (%[p]), %%ymm6\n\t"
        "vmovdqa 7*%c[blockSize]     (%[p]), %%ymm7\n\t"
/*
        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm3, %%ymm2, %%ymm2\n\t"
        "vpaddd %%ymm5, %%ymm4, %%ymm4\n\t"
        "vpaddd %%ymm7, %%ymm6, %%ymm6\n\t"

        "vpaddd %%ymm2, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm6, %%ymm4, %%ymm4\n\t"

        "vpaddd %%ymm4, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm8, %%ymm0, %%ymm8\n\t"
*/
        "add $32, %[p]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        "jne 2b\n\t"

        "add $%c[blockSize7], %[p]\n\t"
        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vphaddd %%ymm8, %%ymm8, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [blockSize7] "i" (blockSize*7), [bufSize] "i" (bufSize)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "xmm1"
    );

    return sum;
}

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test8x1( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n\t"
        "vpxor %%ymm2, %%ymm2, %%ymm2\n\t"
        "vpxor %%ymm3, %%ymm3, %%ymm3\n\t"
        "vpxor %%ymm4, %%ymm4, %%ymm4\n\t"
        "vpxor %%ymm5, %%ymm5, %%ymm5\n\t"
        "vpxor %%ymm6, %%ymm6, %%ymm6\n\t"
        "vpxor %%ymm7, %%ymm7, %%ymm7\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"

        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        "2:\n\t"
        "vpaddd                     (%[p]), %%ymm0, %%ymm0\n\t"
        "vpaddd 1*%c[blockSize]     (%[p]), %%ymm1, %%ymm1\n\t"
        "vpaddd 2*%c[blockSize]     (%[p]), %%ymm2, %%ymm2\n\t"
        "vpaddd 3*%c[blockSize]     (%[p]), %%ymm3, %%ymm3\n\t"
        "vpaddd 4*%c[blockSize]     (%[p]), %%ymm4, %%ymm4\n\t"
        "vpaddd 5*%c[blockSize]     (%[p]), %%ymm5, %%ymm5\n\t"
        "vpaddd 6*%c[blockSize]     (%[p]), %%ymm6, %%ymm6\n\t"
        "vpaddd 7*%c[blockSize]     (%[p]), %%ymm7, %%ymm7\n\t"
        "add $32, %[p]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        "jne 2b\n\t"

        "add $%c[blockSize7], %[p]\n\t"
        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm3, %%ymm2, %%ymm2\n\t"
        "vpaddd %%ymm5, %%ymm4, %%ymm4\n\t"
        "vpaddd %%ymm7, %%ymm6, %%ymm6\n\t"

        "vpaddd %%ymm2, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm6, %%ymm4, %%ymm4\n\t"

        "vpaddd %%ymm4, %%ymm0, %%ymm0\n\t"

        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [blockSize7] "i" (blockSize*7), [bufSize] "i" (bufSize)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "xmm0", "xmm1"
    );

    return sum;
}

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test2x2( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n\t"
        "vpxor %%ymm2, %%ymm2, %%ymm2\n\t"
        "vpxor %%ymm3, %%ymm3, %%ymm3\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"

        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        "2:\n\t"
        "vpaddd                   (%[p]), %%ymm0, %%ymm0\n\t"
        "vpaddd                 32(%[p]), %%ymm1, %%ymm1\n\t"
        "vpaddd %c[blockSize]     (%[p]), %%ymm2, %%ymm2\n\t"
        "vpaddd %c[blockSize] + 32(%[p]), %%ymm3, %%ymm3\n\t"
        "add $64, %[p]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        "jne 2b\n\t"

        "add $%c[blockSize], %[p]\n\t"
        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm3, %%ymm2, %%ymm2\n\t"

        "vpaddd %%ymm2, %%ymm0, %%ymm0\n\t"

        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [bufSize] "i" (bufSize)
        : "ymm0", "ymm1", "ymm2", "ymm3", "xmm0", "xmm1"
    );

    return sum;
}

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test2x4( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n\t"
        "vpxor %%ymm2, %%ymm2, %%ymm2\n\t"
        "vpxor %%ymm3, %%ymm3, %%ymm3\n\t"
        "vpxor %%ymm4, %%ymm4, %%ymm4\n\t"
        "vpxor %%ymm5, %%ymm5, %%ymm5\n\t"
        "vpxor %%ymm6, %%ymm6, %%ymm6\n\t"
        "vpxor %%ymm7, %%ymm7, %%ymm7\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"

        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        "2:\n\t"
        "vpaddd                   (%[p]), %%ymm0, %%ymm0\n\t"
        "vpaddd                 32(%[p]), %%ymm1, %%ymm1\n\t"
        "vpaddd                 64(%[p]), %%ymm2, %%ymm2\n\t"
        "vpaddd                 96(%[p]), %%ymm3, %%ymm3\n\t"
        "vpaddd %c[blockSize]     (%[p]), %%ymm4, %%ymm4\n\t"
        "vpaddd %c[blockSize] + 32(%[p]), %%ymm5, %%ymm5\n\t"
        "vpaddd %c[blockSize] + 64(%[p]), %%ymm6, %%ymm6\n\t"
        "vpaddd %c[blockSize] + 96(%[p]), %%ymm7, %%ymm7\n\t"
        "add $128, %[p]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        "jne 2b\n\t"

        "add $%c[blockSize], %[p]\n\t"
        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm3, %%ymm2, %%ymm2\n\t"
        "vpaddd %%ymm5, %%ymm4, %%ymm4\n\t"
        "vpaddd %%ymm7, %%ymm6, %%ymm6\n\t"

        "vpaddd %%ymm2, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm6, %%ymm4, %%ymm4\n\t"

        "vpaddd %%ymm4, %%ymm0, %%ymm0\n\t"

        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [bufSize] "i" (bufSize)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "xmm0", "xmm1"
    );

    return sum;
}

template<int bufSize, int blockSize>
[[gnu::always_inline]] inline uint32_t test2x4_lea2( const char* p )
{
    uint32_t sum = 0;
    const char* blockEnd;
    const char* bufEnd;
    const char* p2 = p;
    asm volatile (
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vpxor %%ymm1, %%ymm1, %%ymm1\n\t"
        "vpxor %%ymm2, %%ymm2, %%ymm2\n\t"
        "vpxor %%ymm3, %%ymm3, %%ymm3\n\t"
        "vpxor %%ymm4, %%ymm4, %%ymm4\n\t"
        "vpxor %%ymm5, %%ymm5, %%ymm5\n\t"
        "vpxor %%ymm6, %%ymm6, %%ymm6\n\t"
        "vpxor %%ymm7, %%ymm7, %%ymm7\n\t"
        "lea %c[bufSize](%[p]), %[bufEnd]\n\t"
        "lea %c[blockSize](%[p2]), %[p2]\n\t"

        "1:\n\t"
        "lea %c[blockSize](%[p]), %[blockEnd]\n\t"

        "2:\n\t"
        "vpaddd                   (%[p]), %%ymm0, %%ymm0\n\t"
        "vpaddd                 32(%[p]), %%ymm1, %%ymm1\n\t"
        "vpaddd                 64(%[p]), %%ymm2, %%ymm2\n\t"
        "vpaddd                 96(%[p]), %%ymm3, %%ymm3\n\t"
        "vpaddd                  (%[p2]), %%ymm4, %%ymm4\n\t"
        "vpaddd                32(%[p2]), %%ymm5, %%ymm5\n\t"
        "vpaddd                64(%[p2]), %%ymm6, %%ymm6\n\t"
        "vpaddd                96(%[p2]), %%ymm7, %%ymm7\n\t"
        "add $128, %[p]\n\t"
        "add $128, %[p2]\n\t"
        "cmp %[blockEnd], %[p]\n\t"
        "jne 2b\n\t"

        "add $%c[blockSize], %[p]\n\t"
        "add $%c[blockSize], %[p2]\n\t"
        "cmp %[bufEnd], %[p]\n\t"
        "jne 1b\n\t"

        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm3, %%ymm2, %%ymm2\n\t"
        "vpaddd %%ymm5, %%ymm4, %%ymm4\n\t"
        "vpaddd %%ymm7, %%ymm6, %%ymm6\n\t"

        "vpaddd %%ymm2, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm6, %%ymm4, %%ymm4\n\t"

        "vpaddd %%ymm4, %%ymm0, %%ymm0\n\t"

        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vphaddd %%ymm0, %%ymm0, %%ymm0\n\t"
        "vextracti128 $0x1, %%ymm0, %%xmm1\n\t"
        "vpaddd %%xmm1, %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[sum]\n\t"

        : [sum] "=r" (sum), [p] "+r" (p), [p2] "+r" (p2), [blockEnd] "=r" (blockEnd), [bufEnd] "=r" (bufEnd)
        : [blockSize] "i" (blockSize), [bufSize] "i" (bufSize)
        : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "xmm0", "xmm1"
    );

    return sum;
}

//#define test_fun( x ) test1x1<8*1024, 8*1024>( x )
//#define test_fun( x ) test1x2<8*1024, 8*1024>( x )
//#define test_fun( x ) test2x2<8*1024, 4*1024>( x ) // 188.53
//#define test_fun( x ) test2x4<8*1024, 4*1024>( x )
//#define test_fun( x ) test4x1<8*1024, 2*1024>( x ) // 190.42
#define test_fun( x ) test4x1_mov<8*1024, 1024>( x ) // 121.50, 193.14
//#define test_fun( x ) test2x1_mov<8*1024, 2*1024>( x ) 
//#define test_fun( x ) test4x2<8*1024, 2*1024>( x )
//#define test_fun( x ) test4x4<8*1024, 2*1024>( x )
//#define test_fun( x ) test8x1<8*1024, 1*1024>( x )
//#define test_fun( x ) test8x2<8*1024, 1*1024>( x )
//#define test_fun( x ) test16x1<8*1024, 512>( x ) // 178.97
//#define test_fun( x ) test16x1_mov<8*1024, 512>( x ) // 117.25
//#define test_fun( x ) test8x1_mov<8*1024, 512>( x ) // 118, 187

int main( int argc, const char** argv )
{
    uint32_t cpu;
    rdtscp( cpu );
    pinToCore( cpu );
    enable_counters( cpu );

    uint32_t* buf = ( uint32_t* ) mmap( nullptr, 1<<21, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0 );
    for( size_t i=0; i<bufSize; ++i )
        buf[i] = i;

    vector<PmcConfig>& pmcConfigs = pmcConfigsRaptorLakePCore;

    uint32_t sum = 0;
    uint32_t sum1 = 0;
    for( int i=0; i<1024*2; ++i )
        sum1 += i;
    cout<<sum1<<endl;
    const size_t N = 100'000;
    const size_t pmcCnt = 4;
    array<uint64_t, 2> t = {};
    vector<array<uint64_t, 2>> pmcFix( 3 );
    vector<array<uint64_t, 2>> pmc( pmcConfigs.size() );
    for( size_t i = 0; i < pmcConfigs.size(); i+=pmcCnt )
    {
        size_t n=pmcCnt;
        for( size_t j = 0; j < n; ++j )
        {
            if( i + j == pmcConfigs.size() )
            {
                n = j;
                break;
            }
            setPmc( cpu, j, pmcConfigs[ i + j] );
        }

        for( int i=0; i<1000; ++i )
            sum = test_fun( ( const char* )buf );

        if( i == 0 )
        {
            t[0] = rdtsc();
            for( size_t j = 0; j < 3; ++j )
                pmcFix[j][0] = readFixPmc( j );
        }
        for( size_t j = 0; j < n; ++j )
            pmc[i + j][0] = readPmc( j );

        for( uint64_t i=0; i<N; ++i )
            sum = test_fun( ( const char* )buf );

        for( size_t j = 0; j < n; ++j )
            pmc[i + j][1] = readPmc( j );
        if( i == 0 )
        {
            t[1] = rdtsc();
            for( size_t j = 0; j < 3; ++j )
                pmcFix[j][1] = readFixPmc( j );
        }
    }

    munmap( buf, 1<<21 );

    for( size_t i = 0; i < pmcConfigs.size(); ++i )
    {
        if( pmc[i][1] - pmc[i][0] > 1000 )
            printf( "PMC%lu:     %12.2f %s\n", i%pmcCnt, 1.0*( pmc[i][1] - pmc[i][0] )/N, pmcConfigs[i].name );
    }

    for( size_t i = 0; i < 3; ++i )
        printf( "FIX_PMC%lu: %12.2f %s\n", i, 1.0*( pmcFix[i][1] - pmcFix[i][0] )/N, pmcFixNotes[i] );
    printf( "TICK:     %12.2f\n", 1.0*( t[1] - t[0] )/N );
    printf( "Sum: %u\n", sum );

    return 0;
}


