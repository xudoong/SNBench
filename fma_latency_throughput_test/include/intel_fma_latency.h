
#pragma once
#include <immintrin.h>
#include "papi.h"
#include "include.h"

namespace op
{
const std::string NAME="FMA";

double run_serial(double a, double b, double c, int repeat) {
    double a0 = a;
    __m128d v_a, v_b, v_c;

    v_a = _mm_load_sd(&a);
    v_b = _mm_load_sd(&b);
    v_c = _mm_load_sd(&c);
    /* check for correctness */
    v_a = _mm_fmadd_sd(v_a, v_b, v_c);
    a = _mm_cvtsd_f64(v_a);
    assert(a == a0 * b + c);
    /* set a back to a0 */
    a += LARGE_VALUE;
    a -= LARGE_VALUE;
    a += a0;
    assert(a == a0);

    v_a = _mm_load_sd(&a);
    __m128d v_large = _mm_set1_pd(LARGE_VALUE);
    __m128d v_a0 = _mm_set1_pd(a);

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        v_a = _mm_fmadd_sd(v_a, v_b, v_c);
        v_a = _mm_add_sd(v_a, v_large);
        v_a = _mm_sub_sd(v_a, v_large);
        v_a = _mm_add_sd(v_a, v_a0);
    }
    v_a = _mm_fmadd_sd(v_a, v_b, v_c);
    a = _mm_cvtsd_f64(v_a);
    return a;
}

double run_sse_128(double a, double b, double c, int repeat) {
    __m128d v_a, v_b, v_c;
    v_a = _mm_set1_pd(a);
    v_b = _mm_set1_pd(b);
    v_c = _mm_set1_pd(c);

    __m128d v_large = _mm_set1_pd(LARGE_VALUE);
    __m128d v_a0 = _mm_set1_pd(a);

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        v_a = _mm_fmadd_pd(v_a, v_b, v_c);
        v_a = _mm_add_pd(v_a, v_large);
        v_a = _mm_sub_pd(v_a, v_large);
        v_a = _mm_add_pd(v_a, v_a0);
    }

    double tmp[2];
    _mm_store_pd(tmp, v_a);
    a = tmp[0];
    return a;
}

double run_avx_256(double a, double b, double c, int repeat) {
    __m256d v_a, v_b, v_c;
    v_a = _mm256_set1_pd(a);
    v_b = _mm256_set1_pd(b);
    v_c = _mm256_set1_pd(c);

    __m256d v_large = _mm256_set1_pd(LARGE_VALUE);
    __m256d v_a0 = _mm256_set1_pd(a);

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        v_a = _mm256_fmadd_pd(v_a, v_b, v_c);
        v_a = _mm256_add_pd(v_a, v_large);
        v_a = _mm256_sub_pd(v_a, v_large);
        v_a = _mm256_add_pd(v_a, v_a0);
    }

    double tmp[4];
    _mm256_store_pd(tmp, v_a);
    a = tmp[0];
    return a;
}

double run_avx_512(double a, double b, double c, int repeat) {
    __m512d v_a, v_b, v_c;
    v_a = _mm512_set1_pd(a);
    v_b = _mm512_set1_pd(b);
    v_c = _mm512_set1_pd(c);
    __m512d v_large = _mm512_set1_pd(LARGE_VALUE);
    __m512d v_a0 = _mm512_set1_pd(a);

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        v_a = _mm512_fmadd_pd(v_a, v_b, v_c);
        v_a = _mm512_add_pd(v_a, v_large);
        v_a = _mm512_sub_pd(v_a, v_large);
        v_a = _mm512_add_pd(v_a, v_a0);
    }

    double tmp[8];
    _mm512_store_pd(tmp, v_a);
    a = tmp[0];
    return a;
}

double comp_op(double a, double b, double c) {
    __m128d v_a, v_b, v_c;

    v_a = _mm_load_sd(&a);
    v_b = _mm_load_sd(&b);
    v_c = _mm_load_sd(&c);
    /* check for correctness */
    v_a = _mm_fmadd_sd(v_a, v_b, v_c);
    a = _mm_cvtsd_f64(v_a);
    return a;
}

std::string get_input_normal_subnormal_format(double a, double b, double c) {
    return fpclassify(comp_op(a, b, c)) + fpclassify(a * b) + fpclassify(a) + fpclassify(b) + fpclassify(c);
};

}

