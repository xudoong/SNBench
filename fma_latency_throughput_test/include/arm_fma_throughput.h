
#pragma once
#include <arm_neon.h>
#include <arm_sve.h>
#include "include.h"

namespace op
{
const std::string NAME="FMA";

double run_serial(double a, double b, double c, int repeat) {
    float64x2_t v_a, v_b, v_c;
    v_a = vdupq_n_f64(a);
    v_b = vdupq_n_f64(b);
    v_c = vdupq_n_f64(c);

    asm volatile (
        "mov d2, %[a].2d[0]\n\t"
        "mov d3, %[b].2d[0]\n\t"
        "mov d4, %[c].2d[0]\n\t"
        :
        : [a] "w" (v_a), [b] "w" (v_b), [c] "w" (v_c) 
        : "d2", "d3", "d4"
    );

    for (int i = 0; i < repeat; i += 8000) {
        asm volatile (
            DUP_8000("fmadd d1, d2, d3, d4\n\t")
            :
            :
            : "d1", "d2", "d3", "d4"
        );
    }


    // check for correctness
    float64x2_t v_zero = vdupq_n_f64(0);
    asm volatile(
        "fadd %[a].2d, %[zero].2d, v1.2d\n\t"
        : [a] "=w" (v_a)
        : [zero] "w" (v_zero)
        : "v1"
    );
    double tmp[2];
    vst1q_f64(tmp, v_a);
    assert(tmp[0] != 0 && tmp[0] == a * b + c);
    return 0;
}

double run_neon_128(double a, double b, double c, int repeat) {
    float64x2_t v_a, v_b, v_c;
    v_a = vdupq_n_f64(a);
    v_b = vdupq_n_f64(b);
    v_c = vdupq_n_f64(c);

    for (int i = 0; i < repeat; i += 8000) {
        asm volatile(
            DUP_1000(
                "dup v1.2d, %[c].d[0]\n\t"
                "dup v2.2d, %[c].d[0]\n\t"
                "dup v3.2d, %[c].d[0]\n\t"
                "dup v4.2d, %[c].d[0]\n\t"
                "dup v5.2d, %[c].d[0]\n\t"
                "dup v6.2d, %[c].d[0]\n\t"
                "dup v7.2d, %[c].d[0]\n\t"
                "dup v8.2d, %[c].d[0]\n\t"
                "fmla v1.2d, %[a].2d, %[b].2d\n\t"
                "fmla v2.2d, %[a].2d, %[b].2d\n\t"
                "fmla v3.2d, %[a].2d, %[b].2d\n\t"
                "fmla v4.2d, %[a].2d, %[b].2d\n\t"
                "fmla v5.2d, %[a].2d, %[b].2d\n\t"
                "fmla v6.2d, %[a].2d, %[b].2d\n\t"
                "fmla v7.2d, %[a].2d, %[b].2d\n\t"
                "fmla v8.2d, %[a].2d, %[b].2d\n\t"
            )
            : 
            : [a] "w" (v_a), [b] "w" (v_b), [c] "w" (v_c)
            : "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8"
        );
    }

    // check for correctness
    float64x2_t v_zero = vdupq_n_f64(0);
    asm volatile(
        "fadd %[a].2d, %[zero].2d, v1.2d\n\t"
        : [a] "=w" (v_a)
        : [zero] "w" (v_zero)
        : "v1"
    );
    double tmp[2];
    vst1q_f64(tmp, v_a);
    assert(tmp[0] != 0 && tmp[0] == a * b + c);
    return 0;
}

double run_sve(double a, double b, double c, int repeat) {
    svfloat64_t v_a = svdup_f64(a);
    svfloat64_t v_b = svdup_f64(b);
    svfloat64_t v_c = svdup_f64(c);
    svbool_t pg = svptrue_b64();

    for (int i = 0; i < repeat; i += 4000) {
        asm volatile(
                DUP_1000(
                    "mov z1.d, %[c].d\n"
                    "mov z2.d, %[c].d\n"
                    "mov z3.d, %[c].d\n"
                    "mov z4.d, %[c].d\n"
                    // "mov z5.d, %[c].d\n"
                    // "mov z6.d, %[c].d\n"
                    // "mov z7.d, %[c].d\n"
                    // "mov z8.d, %[c].d\n"
                    "fmla z1.d, %[p]/m, %[a].d, %[b].d\n"
                    "fmla z2.d, %[p]/m, %[a].d, %[b].d\n"
                    "fmla z3.d, %[p]/m, %[a].d, %[b].d\n"
                    "fmla z4.d, %[p]/m, %[a].d, %[b].d\n"
                    // "fmla z5.d, %[p]/m, %[a].d, %[b].d\n"
                    // "fmla z6.d, %[p]/m, %[a].d, %[b].d\n"
                    // "fmla z7.d, %[p]/m, %[a].d, %[b].d\n"
                    // "fmla z8.d, %[p]/m, %[a].d, %[b].d\n"
                )
                :
                : [a] "w" (v_a), [b] "w" (v_b), [c] "w" (v_c), [p] "Upa" (pg)
                : "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8"
            );
    }
    return 0;
}

double comp_op(double a, double b, double c) {
    return a * b + c;
}

std::string get_input_normal_subnormal_format(double a, double b, double c) {
    return fpclassify(comp_op(a, b, c)) + fpclassify(a * b) + fpclassify(a) + fpclassify(b) + fpclassify(c);
};

};
