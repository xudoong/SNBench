
#pragma once
#include <arm_neon.h>
#include <arm_sve.h>
#include "include.h"

namespace op
{
const std::string NAME="FMA";

double run_serial(double a, double b, double c, int repeat) {
    double a0 = a;

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        a = a * b + c;
        a += LARGE_VALUE;
        a -= LARGE_VALUE;
        a += a0;
    }
    return a;
}

double comp_op(double a, double b, double c) {
    return a * b + c;
}

double run_neon_128(double a, double b, double c, int repeat) {
    double a0 = a;
    float64x2_t v_a, v_b, v_c, v_l, v_a0;
    v_a = vdupq_n_f64(a);
    v_b = vdupq_n_f64(b);
    v_c = vdupq_n_f64(c);
    v_l = vdupq_n_f64(LARGE_VALUE);
    v_a0 = vdupq_n_f64(a);

    // correctness check
    {
        double tmp[2];
        v_a = vdupq_n_f64(a);
        vst1q_f64(tmp, v_a);
        assert(tmp[0] == a);
        assert(tmp[1] == a);

        v_b = vdupq_n_f64(b);
        vst1q_f64(tmp, v_b);
        assert(tmp[0] == b);
        assert(tmp[1] == b);
        
        v_c = vdupq_n_f64(c);
        vst1q_f64(tmp, v_c);
        assert(tmp[0] == c);
        assert(tmp[1] == c);

        v_a = vfmaq_f64(v_c, v_a, v_b); 
        vst1q_f64(tmp, v_a);
        assert(tmp[0] == comp_op(a, b, c));
        assert(tmp[1] == comp_op(a, b, c));
    }

    v_a = v_a0;

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        v_a = vfmaq_f64(v_c, v_a, v_b); 
        v_a = vaddq_f64(v_a, v_l);
        v_a = vsubq_f64(v_a, v_l);
        v_a = vaddq_f64(v_a, v_a0);
    }

    // check for correctness
    v_a = vfmaq_f64(v_c, v_a, v_b); 
    double tmp[2];
    vst1q_f64(tmp, v_a);
    a = tmp[0];
    assert(a == a0 * b + c);
    return a;
}

double run_sve(double a, double b, double c, int repeat) {
    svfloat64_t v_a = svdup_f64(a);
    svfloat64_t v_b = svdup_f64(b);
    svfloat64_t v_c = svdup_f64(c);
    svfloat64_t v_l = svdup_f64(LARGE_VALUE);
    svfloat64_t v_c0 = v_c;
    svbool_t pg = svptrue_b64();
    const int vec_len = svlen_f64(v_a);

    // correctness check
    {
        double tmp[vec_len];
        svst1_f64(pg, tmp, v_a);
        for (int i = 0; i < vec_len; i++) {
            assert(tmp[i] == a);
        }
        
        asm volatile(
            "fmla %[c].d, %[p]/m, %[a].d, %[b].d\n" 
            : [c] "+w" (v_c) 
            : [a] "w" (v_a), [b] "w" (v_b), [p] "Upa" (pg)
            :
        );
        svst1_f64(pg, tmp, v_c);
        for (int i = 0; i < vec_len; i++) {
            assert(tmp[i] == comp_op(a, b, c));
        }
    }

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        asm volatile(
            "fmla %[c].d, %[p]/m, %[a].d, %[b].d\n" 
            "fadd %[c].d, %[p]/m, %[c].d, %[l].d\n" 
            "fsub %[c].d, %[p]/m, %[c].d, %[l].d\n" 
            "fadd %[c].d, %[p]/m, %[c].d, %[c0].d\n" 
            : [c] "+w" (v_c) 
            : [a] "w" (v_a), [b] "w" (v_b), [l] "w" (v_l), [c0] "w" (v_c0), [p] "Upa" (pg)
            :
        );
    }

    double tmp[vec_len];
    svst1_f64(pg, tmp, v_c);
    return tmp[0];
}   

std::string get_input_normal_subnormal_format(double a, double b, double c) {
    return fpclassify(comp_op(a, b, c)) + fpclassify(a * b) + fpclassify(a) + fpclassify(b) + fpclassify(c);
};

};
