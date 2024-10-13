
#pragma once
#include <arm_neon.h>
#include <arm_sve.h>
#include "include.h"

namespace op
{
const std::string NAME="SQRT";
const bool has_neon_128 = true;

double run_serial(double a, double b, int repeat) {
    float64x2_t v_a, v_b;
    v_a = vdupq_n_f64(a);

    asm volatile (
        "mov d2, %[a].2d[0]\n"
        :
        : [a] "w" (v_a)
        : "d2"
    );

    for (int i = 0; i < repeat; i += UNROOL) {
        asm volatile (
            DUP_X("fsqrt d1, d2\n")
            :
            :
            : "d1", "d2"
        );
    }
    return 0;
}

double run_neon_128(double a, double b, int repeat) {
    float64x2_t v_a, v_out, v_out2;
    v_a = vdupq_n_f64(a);

    for (int i = 0; i < repeat; i += UNROOL) {
        asm volatile(
                DUP_X("fsqrt %[out].2d, %[a].2d\n")
                : [out] "=w" (v_out) 
                : [a] "w" (v_a)
                :
            );
    }
    return 0;
}

double run_sve(double a, double b, int repeat) {
    svfloat64_t v_a = svdup_f64(a);
    svfloat64_t v_b = svdup_f64(b);
    svfloat64_t v_out, v_out2, v_out3, v_out4;
    svbool_t pg = svptrue_b64();

    for (int i = 0; i < repeat; i += 4000) {
        asm volatile(
                DUP_1000(
                    "fsqrt %[out].d, %[p]/m, %[a].d\n"
                    "fsqrt %[out2].d, %[p]/m, %[a].d\n"    
                    "fsqrt %[out3].d, %[p]/m, %[a].d\n"    
                    "fsqrt %[out4].d, %[p]/m, %[a].d\n"    
                )
                : [out] "=w" (v_out), [out2] "=w" (v_out2), [out3] "=w" (v_out3) ,[out4] "=w" (v_out4) 
                : [a] "w" (v_a), [p] "Upa" (pg)
                :
            );
    }
    return 0;
}

double comp_op(double a, double b) {
    return sqrt(a);
}

std::string get_input_normal_subnormal_format(double a, double b) {
    return fpclassify(comp_op(a, b)) + fpclassify(a);
};

};
