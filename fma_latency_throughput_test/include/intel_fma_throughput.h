
#pragma once
#include <immintrin.h>
#include "papi.h"
#include "include.h"

namespace op
{
const std::string NAME="FMA";

double run_serial(double a, double b, double c, int repeat) {
    asm volatile (
        "vmovsd %[a_value], %%xmm0\n\t" 
        "vmovsd %[b_value], %%xmm14\n\t" 
        "vmovsd %[c_value], %%xmm15\n\t" 
        : 
        : [a_value] "m" (a), [b_value] "m" (b), [c_value] "m" (c)
        : "%xmm0", "%xmm14", "%xmm15"
    );

    for (int i = 0; i < repeat; i+=8000) {
        asm volatile (
            DUP_1000(
                "vmovapd %%xmm0, %%xmm1\n\t" 
                "vmovapd %%xmm0, %%xmm2\n\t" 
                "vmovapd %%xmm0, %%xmm3\n\t" 
                "vmovapd %%xmm0, %%xmm4\n\t" 
                "vmovapd %%xmm0, %%xmm5\n\t" 
                "vmovapd %%xmm0, %%xmm6\n\t" 
                "vmovapd %%xmm0, %%xmm7\n\t" 
                "vmovapd %%xmm0, %%xmm8\n\t" 
                "vfmadd132sd %%xmm14, %%xmm15, %%xmm1\n\t"
                "vfmadd132sd %%xmm14, %%xmm15, %%xmm2\n\t"
                "vfmadd132sd %%xmm14, %%xmm15, %%xmm3\n\t"
                "vfmadd132sd %%xmm14, %%xmm15, %%xmm4\n\t"
                "vfmadd132sd %%xmm14, %%xmm15, %%xmm5\n\t"  
                "vfmadd132sd %%xmm14, %%xmm15, %%xmm6\n\t"
                "vfmadd132sd %%xmm14, %%xmm15, %%xmm7\n\t"
                "vfmadd132sd %%xmm14, %%xmm15, %%xmm8\n\t"
            )
            : 
            : 
            : "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7", "%xmm8", "%xmm9", "%xmm10", "%xmm11", "%xmm12", "%xmm13", "%xmm14", "%xmm15"
        );
    }
    return 0;
}

double run_sse_128(double a, double b, double c, int repeat) {
    __m128d v_a, v_b, v_c;
    v_a = _mm_set1_pd(a);
    v_b = _mm_set1_pd(b);
    v_c = _mm_set1_pd(c);

    for (int i = 0; i < repeat; i+=8000) {
        asm volatile (
            DUP_1000(
                "vmovapd %[v_a], %%xmm1\n\t" 
                "vmovapd %[v_a], %%xmm2\n\t" 
                "vmovapd %[v_a], %%xmm3\n\t" 
                "vmovapd %[v_a], %%xmm4\n\t" 
                "vmovapd %[v_a], %%xmm5\n\t" 
                "vmovapd %[v_a], %%xmm6\n\t" 
                "vmovapd %[v_a], %%xmm7\n\t" 
                "vmovapd %[v_a], %%xmm8\n\t" 
                "vfmadd132pd %[v_b], %[v_c], %%xmm1\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%xmm2\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%xmm3\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%xmm4\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%xmm5\n\t"  
                "vfmadd132pd %[v_b], %[v_c], %%xmm6\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%xmm7\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%xmm8\n\t"
            )
            : 
            : [v_a] "v" (v_a), [v_b] "v" (v_b), [v_c] "v" (v_c)
            : "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7", "%xmm8"
        );
    }
    return 0;
}

double run_avx_256(double a, double b, double c, int repeat) {
    __m256d v_a, v_b, v_c;
    v_a = _mm256_set1_pd(a);
    v_b = _mm256_set1_pd(b);
    v_c = _mm256_set1_pd(c);

    for (int i = 0; i < repeat; i+=8000) {
        asm volatile (
            DUP_1000(
                "vmovapd %[v_a], %%ymm1\n\t" 
                "vmovapd %[v_a], %%ymm2\n\t" 
                "vmovapd %[v_a], %%ymm3\n\t" 
                "vmovapd %[v_a], %%ymm4\n\t" 
                "vmovapd %[v_a], %%ymm5\n\t" 
                "vmovapd %[v_a], %%ymm6\n\t" 
                "vmovapd %[v_a], %%ymm7\n\t" 
                "vmovapd %[v_a], %%ymm8\n\t" 
                "vfmadd132pd %[v_b], %[v_c], %%ymm1\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%ymm2\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%ymm3\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%ymm4\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%ymm5\n\t"  
                "vfmadd132pd %[v_b], %[v_c], %%ymm6\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%ymm7\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%ymm8\n\t"
            )
            : 
            : [v_a] "v" (v_a), [v_b] "v" (v_b), [v_c] "v" (v_c)
            : "%ymm1", "%ymm2", "%ymm3", "%ymm4", "%ymm5", "%ymm6", "%ymm7", "%ymm8"
        );
    }
    return 0;
}

double run_avx_512(double a, double b, double c, int repeat) {
    __m512d v_a, v_b, v_c;
    v_a = _mm512_set1_pd(a);
    v_b = _mm512_set1_pd(b);
    v_c = _mm512_set1_pd(c);

    for (int i = 0; i < repeat; i+=8000) {
        asm volatile (
            DUP_1000(
                "vmovapd %[v_a], %%zmm1\n\t" 
                "vmovapd %[v_a], %%zmm2\n\t" 
                "vmovapd %[v_a], %%zmm3\n\t" 
                "vmovapd %[v_a], %%zmm4\n\t" 
                "vmovapd %[v_a], %%zmm5\n\t" 
                "vmovapd %[v_a], %%zmm6\n\t" 
                "vmovapd %[v_a], %%zmm7\n\t" 
                "vmovapd %[v_a], %%zmm8\n\t" 
                "vfmadd132pd %[v_b], %[v_c], %%zmm1\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%zmm2\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%zmm3\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%zmm4\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%zmm5\n\t"  
                "vfmadd132pd %[v_b], %[v_c], %%zmm6\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%zmm7\n\t"
                "vfmadd132pd %[v_b], %[v_c], %%zmm8\n\t"
            )
            : 
            : [v_a] "v" (v_a), [v_b] "v" (v_b), [v_c] "v" (v_c)
            : "%zmm1", "%zmm2", "%zmm3", "%zmm4", "%zmm5", "%zmm6", "%zmm7", "%zmm8"
        );
    }
    return 0;
}

double comp_op(double a, double b, double c) {
    __m128d v_a, v_b, v_c;

    v_a = _mm_load_sd(&a);
    v_b = _mm_load_sd(&b);
    v_c = _mm_load_sd(&c);
    v_a = _mm_fmadd_sd(v_a, v_b, v_c);
    a = _mm_cvtsd_f64(v_a);
    return a;
}

std::string get_input_normal_subnormal_format(double a, double b, double c) {
    return fpclassify(comp_op(a, b, c)) + fpclassify(a * b) + fpclassify(a) + fpclassify(b) + fpclassify(c);
};

}

