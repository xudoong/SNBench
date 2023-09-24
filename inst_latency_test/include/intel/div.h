
#pragma once
#include <immintrin.h>
#include "papi.h"
#include "include.h"

namespace op
{
const std::string NAME="DIV";

double run_serial(double a, double b, int repeat) {
    double a0 = a;

    /* check for correctness */
    a = a / b;
    /* set a back to a0 */
    a += LARGE_VALUE;
    a -= LARGE_VALUE;
    a += a0;
    assert(a == a0);

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        a = a / b;
        a += LARGE_VALUE;
        a -= LARGE_VALUE;
        a += a0;
    }
    return a;
}

double run_sse_128(double a, double b, int repeat) {
    __m128d v_a, v_b;
    v_a = _mm_set1_pd(a);
    v_b = _mm_set1_pd(b);

    __m128d v_large = _mm_set1_pd(LARGE_VALUE);
    __m128d v_a0 = _mm_set1_pd(a);

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        v_a = _mm_div_pd(v_a, v_b);
        v_a = _mm_add_pd(v_a, v_large);
        v_a = _mm_sub_pd(v_a, v_large);
        v_a = _mm_add_pd(v_a, v_a0);
    }

    double tmp[2];
    _mm_store_pd(tmp, v_a);
    a = tmp[0];
    return a;
}

double run_avx_256(double a, double b, int repeat) {
    __m256d v_a, v_b;
    v_a = _mm256_set1_pd(a);
    v_b = _mm256_set1_pd(b);

    __m256d v_large = _mm256_set1_pd(LARGE_VALUE);
    __m256d v_a0 = _mm256_set1_pd(a);

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        v_a = _mm256_div_pd(v_a, v_b);
        v_a = _mm256_add_pd(v_a, v_large);
        v_a = _mm256_sub_pd(v_a, v_large);
        v_a = _mm256_add_pd(v_a, v_a0);
    }

    double tmp[4];
    _mm256_store_pd(tmp, v_a);
    a = tmp[0];
    return a;
}

double run_avx_512(double a, double b, int repeat) {
    __m512d v_a, v_b;
    v_a = _mm512_set1_pd(a);
    v_b = _mm512_set1_pd(b);

    __m512d v_large = _mm512_set1_pd(LARGE_VALUE);
    __m512d v_a0 = _mm512_set1_pd(a);

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        v_a = _mm512_div_pd(v_a, v_b);
        v_a = _mm512_add_pd(v_a, v_large);
        v_a = _mm512_sub_pd(v_a, v_large);
        v_a = _mm512_add_pd(v_a, v_a0);
    }

    double tmp[8];
    _mm512_store_pd(tmp, v_a);
    a = tmp[0];
    return a;
}

double comp_op(double a, double b) {
    return a / b;
}

std::string get_input_normal_subnormal_format(double a, double b) {
    return fpclassify(comp_op(a, b)) + fpclassify(a) + fpclassify(b);
};

}

