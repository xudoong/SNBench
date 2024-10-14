

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
const std::string NAME="POW";
alignas(64) double out[MAX_N];

double run_serial(const double *a, const double *b, int n, int repeat) {
    for (int rp = 0; rp < repeat; rp++) {
        for (int i = 0; i < n; i++) {
#if defined(MATH_LIB_AML)
            out[i] = armpl_pow_f64(a[i], b[i]);
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
double run_neon_128(const double *a, const double *b, int n, int repeat) {
    for (int rp = 0; rp < repeat; rp++) {
        for (int i = 0; i < n; i+=2) {
            float64x2_t v_a = vld1q_f64(a + i);
			float64x2_t v_b = vld1q_f64(b + i);
#if defined(MATH_LIB_AML)
            v_a = armpl_vpowq_f64(v_a, v_b);
#elif defined(MATH_LIB_SLEEF_U10)
            v_a = Sleef_powd2_u10(v_a, v_b);
#elif defined(MATH_LIB_SLEEF_U35)
            v_a = Sleef_powd2_u35(v_a, v_b);
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
            svfloat64_t v_b = svld1_f64(pg, b + i);
#if defined(MATH_LIB_AML)
            v_a = armpl_svpow_f64_x(v_a, v_b, pg);
#elif defined(MATH_LIB_SLEEF_U10)
            v_a = Sleef_powdx_u10sve(v_a, v_b);
#elif defined(MATH_LIB_SLEEF_U35)
            v_a = Sleef_powdx_u35sve(v_a, v_b);
#endif
            svst1_f64(pg, out + i, v_a);
        }
    }
    return out[0];
}

#endif

double comp_op(double a, double b) {
#if defined(MATH_LIB_AML)
            a = armpl_pow_f64(a, b);
#elif defined(MATH_LIB_SLEEF_U10)
            a = Sleef_pow_u10(a, b);
#elif defined(MATH_LIB_SLEEF_U35)
            a = Sleef_pow_u35(a, b);
#else 
            a = pow(a, b);
#endif
    return a;
}


std::string get_input_normal_subnormal_format(double a, double b) {
    return fpclassify(comp_op(a, b)) + fpclassify(a) + fpclassify(b);
};


}

