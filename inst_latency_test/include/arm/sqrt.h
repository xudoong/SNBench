
#pragma once
#include <arm_neon.h>
#include <arm_sve.h>
#include "include.h"

namespace op
{
const std::string NAME="SQRT";

double run_serial(double a, double b, int repeat) {
    double a0 = a;

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        a = sqrt(a);
        a += LARGE_VALUE;
        a -= LARGE_VALUE;
        a += a0;
    }
    return a;
}

double comp_op(double a, double b) {
    return sqrt(a);
}

double run_neon_128(double a, double b, int repeat) {
    float64x2_t v_a, v_b, v_l, v_a0;
    v_a = vdupq_n_f64(a);
    v_b = vdupq_n_f64(b);
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
        
        asm volatile(
            "fsqrt %[a].2d, %[a].2d\n" 
            : [a] "+w" (v_a) 
            : [b] "w" (v_b)
            :
        );
        vst1q_f64(tmp, v_a);
        assert(tmp[0] == comp_op(a, b));
        assert(tmp[1] == comp_op(a, b));
    }



    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        asm volatile(
                "fsqrt %[a].2d, %[a].2d\n" 
                "fadd %[a].2d, %[a].2d, %[l].2d\n" 
                "fsub %[a].2d, %[a].2d, %[l].2d\n" 
                "fadd %[a].2d, %[a].2d, %[a0].2d\n" 
                : [a] "+w" (v_a) 
                : [b] "w" (v_b), [l] "w" (v_l), [a0] "w" (v_a0)
                :
            );
    }

    double tmp[2];
    vst1q_f64(tmp, v_a);
    a = tmp[0];
    return a;
}

double run_sve(double a, double b, int repeat) {
    svfloat64_t v_a = svdup_f64(a);
    svfloat64_t v_b = svdup_f64(b);
    svfloat64_t v_l = svdup_f64(LARGE_VALUE);
    svfloat64_t v_a0 = v_a;
    svbool_t pg = svptrue_b64();
    const int vec_len = svlen_f64(v_a);

    // correctness check
    {
        double tmp[vec_len];
        svst1_f64(pg, tmp, v_a);
        for (int i = 0; i < vec_len; i++) {
            assert(tmp[i] == a);
        }

        svst1_f64(pg, tmp, v_b);
        for (int i = 0; i < vec_len; i++) {
            assert(tmp[i] == b);
        }
        
        asm volatile(
            "fsqrt %[a].d, %[p]/m, %[a].d\n" 
            : [a] "+w" (v_a) 
            : [p] "Upa" (pg)
            :
        );
        svst1_f64(pg, tmp, v_a);
        for (int i = 0; i < vec_len; i++) {
            assert(tmp[i] == comp_op(a, b));
        }
    }

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        asm volatile(
            "fsqrt %[a].d, %[p]/m, %[a].d\n" 
            "fadd %[a].d, %[p]/m, %[a].d, %[l].d\n" 
            "fsub %[a].d, %[p]/m, %[a].d, %[l].d\n" 
            "fadd %[a].d, %[p]/m, %[a].d, %[a0].d\n" 
            : [a] "+w" (v_a) 
            : [l] "w" (v_l), [a0] "w" (v_a0), [p] "Upa" (pg)
            :
        );
    }

    double tmp[vec_len];
    svst1_f64(pg, tmp, v_a);
    return tmp[0];
}

std::string get_input_normal_subnormal_format(double a, double b) {
    return fpclassify(comp_op(a, b)) + fpclassify(a);
};

};
