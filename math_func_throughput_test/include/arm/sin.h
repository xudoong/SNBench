

#pragma once
#include <arm_neon.h>
#include "include.h"
#if defined(MATH_LIB_AML)
extern "C" {
#include "mathlib.h"
}
#elif defined(MATH_LIB_SLEEF_U10) || defined(MATH_LIB_SLEEF_U35)
#include "sleef.h"
#endif

namespace op
{
const std::string NAME="SIN";
alignas(64) double out[MAX_N];

double run_serial(const double *a, const double *b, int n, int repeat) {
    for (int rp = 0; rp < repeat; rp++) {
        for (int i = 0; i < n; i++) {
#if defined(MATH_LIB_AML)
            out[i] = __s_sin(a[i]);
#elif defined(MATH_LIB_SLEEF_U10)
            out[i] = Sleef_sin_u10(a[i]);
#elif defined(MATH_LIB_SLEEF_U35)
            out[i] = Sleef_sin_u35(a[i]);
#else 
            out[i] = sin(a[i]);
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
            v_a = __v_sin(v_a);
#elif defined(MATH_LIB_SLEEF_U10)
            v_a = Sleef_sind2_u10(v_a);
#elif defined(MATH_LIB_SLEEF_U35)
            v_a = Sleef_sind2_u35(v_a);
#endif
            vst1q_f64(out + i, v_a);
        }
    }
    return out[0];
}
#endif

double comp_op(double a, double b) {
#if defined(MATH_LIB_AML)
            a = __s_sin(a);
#elif defined(MATH_LIB_SLEEF_U10)
            a = Sleef_sin_u10(a);
#elif defined(MATH_LIB_SLEEF_U35)
            a = Sleef_sin_u35(a);
#else 
            a = sin(a);
#endif
    return a;
}


std::string get_input_normal_subnormal_format(double a, double b) {
    return fpclassify(comp_op(a, b)) + fpclassify(a);
};


}

