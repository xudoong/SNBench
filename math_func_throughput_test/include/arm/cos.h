

#pragma once
#include <arm_neon.h>
#include <arm_sve.h>
#include "include.h"
#if defined(MATH_LIB_AML)
extern "C" {
#include "amath.h"
}
#elif defined(MATH_LIB_SLEEF_U10) || defined(MATH_LIB_SLEEF_U35)
#include "sleef.h"
#endif

namespace op
{
const std::string NAME="COS";
alignas(64) double out[MAX_N];

double run_serial(const double *a, const double *b, int n, int repeat) {
    for (int rp = 0; rp < repeat; rp++) {
        for (int i = 0; i < n; i++) {
#if defined(MATH_LIB_AML)
            out[i] = cos(a[i]); // arm performance lib does not have f64 cos, use glibc instead
#elif defined(MATH_LIB_SLEEF_U10)
            out[i] = Sleef_cos_u10(a[i]);
#elif defined(MATH_LIB_SLEEF_U35)
            out[i] = Sleef_cos_u35(a[i]);
#else 
            out[i] = cos(a[i]);
#endif
        }
    }
    return out[0];
}


#ifndef SCALAR_ONLY
double run_neon_128(const double *a, const double *b, int n, int repeat) {
    for (int rp = 0; rp < repeat; rp++) {
        for (int i = 0; i < n; i+=2) {
            float64x2_t v_a = vld1q_f64(a + i);
#if defined(MATH_LIB_AML)
            v_a = armpl_vcosq_f64(v_a);
#elif defined(MATH_LIB_SLEEF_U10)
            v_a = Sleef_cosd2_u10(v_a);
#elif defined(MATH_LIB_SLEEF_U35)
            v_a = Sleef_cosd2_u35(v_a);
#endif
            vst1q_f64(out + i, v_a);
        }
    }
    return out[0];
}

double run_sve(const double *a, const double *b, int n, int repeat) {
    svfloat64_t tmp;
    svbool_t pg = svptrue_b64();
    const int vec_len = svlen_f64(tmp);
    for (int rp = 0; rp < repeat; rp++) {
        for (int i = 0; i < n; i+=vec_len) {
            svfloat64_t v_a = svld1_f64(pg, a + i);
#if defined(MATH_LIB_AML)
            v_a = armpl_svcos_f64_x(v_a, pg);
#elif defined(MATH_LIB_SLEEF_U10)
            v_a = Sleef_cosdx_u10sve(v_a);
#elif defined(MATH_LIB_SLEEF_U35)
            v_a = Sleef_cosdx_u35sve(v_a);
#endif
            svst1_f64(pg, out + i, v_a);
        }
    }
    return out[0];
}

#endif


double comp_op(double a, double b) {
#if defined(MATH_LIB_AML)
            a = cos(a);
#elif defined(MATH_LIB_SLEEF_U10)
            a = Sleef_cos_u10(a);
#elif defined(MATH_LIB_SLEEF_U35)
            a = Sleef_cos_u35(a);
#else 
            a = cos(a);
#endif
    return a;
}


std::string get_input_normal_subnormal_format(double a, double b) {
    return fpclassify(comp_op(a, b)) + fpclassify(a);
};


}

