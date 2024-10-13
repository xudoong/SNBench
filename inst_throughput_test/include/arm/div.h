
#pragma once
#include <arm_neon.h>
#include <arm_sve.h>
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

double run_sve(double a, double b, int repeat) {
    svfloat64_t v_a = svdup_f64(a);
    svfloat64_t v_b = svdup_f64(b);
    svbool_t pg = svptrue_b64();

    for (int i = 0; i < repeat; i += 8000) {
        asm volatile(
                DUP_1000(
                    "mov z1.d, %[a].d\n"
                    "mov z2.d, %[a].d\n"
                    "mov z3.d, %[a].d\n"
                    "mov z4.d, %[a].d\n"
                    "mov z5.d, %[a].d\n"
                    "mov z6.d, %[a].d\n"
                    "mov z7.d, %[a].d\n"
                    "mov z8.d, %[a].d\n"
                    "fdiv z1.d, %[p]/m, z1.d, %[b].d\n"
                    "fdiv z2.d, %[p]/m, z2.d, %[b].d\n"
                    "fdiv z3.d, %[p]/m, z3.d, %[b].d\n"
                    "fdiv z4.d, %[p]/m, z4.d, %[b].d\n"
                    "fdiv z5.d, %[p]/m, z5.d, %[b].d\n"
                    "fdiv z6.d, %[p]/m, z6.d, %[b].d\n"
                    "fdiv z7.d, %[p]/m, z7.d, %[b].d\n"
                    "fdiv z8.d, %[p]/m, z8.d, %[b].d\n"
                )
                :
                : [a] "w" (v_a), [b] "w" (v_b), [p] "Upa" (pg)
                : "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8"
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
