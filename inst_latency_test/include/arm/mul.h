
#pragma once
#include <arm_neon.h>
#include "include.h"
#include "sleef.h"

namespace op
{
const std::string NAME="MUL";
const bool has_neon_128 = true;

double run_serial(double a, double b, int repeat) {
    double a0 = a;

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {
        a = a * b;
        a += LARGE_VALUE;
        a -= LARGE_VALUE;
        a += a0;
    }
    return a;
}

double comp_op(double a, double b) {
    return a * b;
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
            "fmul %[a].2d, %[a].2d, %[b].2d\n" 
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
                "fmul %[a].2d, %[a].2d, %[b].2d\n" 
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

    

std::string get_input_normal_subnormal_format(double a, double b) {
    return fpclassify(comp_op(a, b)) + fpclassify(a) + fpclassify(b);
};

};
