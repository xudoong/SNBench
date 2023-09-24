
#pragma once
#include <immintrin.h>
#include "papi.h"
#include "include.h"

#if defined(MATH_LIB_AMD)
#define AMD_LIBM_VEC_EXPERIMENTAL
#include "amdlibm.h"
#include "amdlibm_vec.h"
#elif defined(MATH_LIB_SLEEF_U10) || defined(MATH_LIB_SLEEF_U35)
#include "sleef.h"
#endif

namespace op
{
const std::string NAME="POW";
alignas(64) double out[MAX_N];

double run_serial(const double *a, const double *b, int n, int repeat) {
    for (int rp = 0; rp < repeat; rp++) {
        for (int i = 0; i < n; i++) {
#if defined(MATH_LIB_AMD)
            out[i] = amd_pow(a[i], b[i]);
#elif defined(MATH_LIB_SLEEF_U10)
            out[i] = Sleef_pow_u10(a[i], b[i]);
#elif defined(MATH_LIB_SLEEF_U35)
            out[i] = Sleef_pow_u35(a[i], b[i]);
#else 
            out[i] = pow(a[i], b[i]);
#endif
        }
    }
    return out[0];
}


#ifndef SCALAR_ONLY
double run_sse_128(const double *a, const double *b, int n, int repeat) {
    for (int rp = 0; rp < repeat; rp++) {
        for (int i = 0; i < n; i+=2) {
            __m128d v_a = _mm_load_pd(a + i);
			__m128d v_b = _mm_load_pd(b + i);
#if defined(MATH_LIB_AMD)
            v_a = amd_vrd2_pow(v_a, v_b);
#elif defined(MATH_LIB_SLEEF_U10)
            v_a = Sleef_powd2_u10(v_a, v_b);
#elif defined(MATH_LIB_SLEEF_U35)
            v_a = Sleef_powd2_u35(v_a, v_b);
#else 
            v_a = _mm_pow_pd(v_a, v_b);
#endif
            _mm_store_pd(out + i, v_a);
        }
    }
    return out[0];
}

double run_avx_256(const double *a, const double *b, int n, int repeat) {
    for (int rp = 0; rp < repeat; rp++) {
        for (int i = 0; i < n; i+=4) {
            __m256d v_a = _mm256_load_pd(a + i);
			__m256d v_b = _mm256_load_pd(b + i);
#if defined(MATH_LIB_AMD)
            v_a = amd_vrd4_pow(v_a, v_b);
#elif defined(MATH_LIB_SLEEF_U10)
            v_a = Sleef_powd4_u10(v_a, v_b);
#elif defined(MATH_LIB_SLEEF_U35)
            v_a = Sleef_powd4_u35(v_a, v_b);
#else 
            v_a = _mm256_pow_pd(v_a, v_b);
#endif
            _mm256_store_pd(out + i, v_a);
        }
    }
    return out[0];
}

double run_avx_512(const double *a, const double *b, int n, int repeat) {
    for (int rp = 0; rp < repeat; rp++) {
        for (int i = 0; i < n; i+=8) {
            __m512d v_a = _mm512_load_pd(a + i);
			__m512d v_b = _mm512_load_pd(b + i);
#if defined(MATH_LIB_AMD)
            v_a = amd_vrd8_pow(v_a, v_b);
#elif defined(MATH_LIB_SLEEF_U10)
            v_a = Sleef_powd8_u10(v_a, v_b);
#elif defined(MATH_LIB_SLEEF_U35)
            v_a = Sleef_powd8_u35(v_a, v_b);
#else 
            v_a = _mm512_pow_pd(v_a, v_b);
#endif
            _mm512_store_pd(out + i, v_a);
        }
    }
    return out[0];
}
#endif

double comp_op(double a, double b) {
#if defined(MATH_LIB_AMD)
    a = amd_pow(a, b);
#elif defined(MATH_LIB_SLEEF_U10)
    a = Sleef_pow_u10(a, b);
#elif defined(MATH_LIB_SLEEF_U35)
    a = Sleef_pow_u35(a);
#else 
    a = pow(a, b);
#endif
    return a;
}

std::string get_input_normal_subnormal_format(double a, double b) {
    return fpclassify(comp_op(a, b)) + fpclassify(a) + fpclassify(b);
};

}
