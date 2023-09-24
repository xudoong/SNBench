
#pragma once
#include <arm_neon.h>
#include "include.h"

namespace op
{
const std::string NAME="DIV";
const bool has_neon_128 = true;

double run_serial(double a, double b, int repeat) {
    float64x2_t v_a, v_b;
    v_a = vdupq_n_f64(a);
    v_b = vdupq_n_f64(b);

    asm volatile (
        "mov d2, %[a].2d[0]\n"
        "mov d3, %[b].2d[0]\n"
        :
        : [a] "w" (v_a), [b] "w" (v_b) 
        : "d2", "d3"
    );

    for (int i = 0; i < repeat; i += UNROOL) {
        asm volatile (
            DUP_X("fdiv d1, d2, d3\n")
            :
            :
            : "d1", "d2", "d3"
        );
    }
    return 0;
}

double run_neon_128(double a, double b, int repeat) {
    float64x2_t v_a, v_b, v_out;
    v_a = vdupq_n_f64(a);
    v_b = vdupq_n_f64(b);

    for (int i = 0; i < repeat; i += UNROOL) {
        asm volatile(
                DUP_X("fdiv %[out].2d, %[a].2d, %[b].2d\n")
                : [out] "=w" (v_out) 
                : [a] "w" (v_a), [b] "w" (v_b)
                :
            );
    }
    return 0;
}

double comp_op(double a, double b) {
    return a / b;
}   

std::string get_input_normal_subnormal_format(double a, double b) {
    return fpclassify(comp_op(a, b)) + fpclassify(a) + fpclassify(b);
};

};
