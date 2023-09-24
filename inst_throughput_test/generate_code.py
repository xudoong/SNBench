import os

def intel():
    intel_op_fmt_str = \
'''
#pragma once
#include <immintrin.h>
#include "papi.h"
#include "include.h"

namespace op
{{
const std::string NAME="{name}";

double run_serial(double a, double b, int repeat) {{
    double a0 = a;

    /* check for correctness */
    a = {op_comp};
    /* set a back to a0 */
    a += LARGE_VALUE;
    a -= LARGE_VALUE;
    a += a0;
    assert(a == a0);

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {{
        a = {op_comp};
        a += LARGE_VALUE;
        a -= LARGE_VALUE;
        a += a0;
    }}
    return a;
}}

double run_sse_128(double a, double b, int repeat) {{
    __m128d v_a, v_b;
    v_a = _mm_set1_pd(a);
    v_b = _mm_set1_pd(b);

    __m128d v_large = _mm_set1_pd(LARGE_VALUE);
    __m128d v_a0 = _mm_set1_pd(a);

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {{
        v_a = _mm_{op_str};
        v_a = _mm_add_pd(v_a, v_large);
        v_a = _mm_sub_pd(v_a, v_large);
        v_a = _mm_add_pd(v_a, v_a0);
    }}

    double tmp[2];
    _mm_store_pd(tmp, v_a);
    a = tmp[0];
    return a;
}}

double run_avx_256(double a, double b, int repeat) {{
    __m256d v_a, v_b;
    v_a = _mm256_set1_pd(a);
    v_b = _mm256_set1_pd(b);

    __m256d v_large = _mm256_set1_pd(LARGE_VALUE);
    __m256d v_a0 = _mm256_set1_pd(a);

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {{
        v_a = _mm256_{op_str};
        v_a = _mm256_add_pd(v_a, v_large);
        v_a = _mm256_sub_pd(v_a, v_large);
        v_a = _mm256_add_pd(v_a, v_a0);
    }}

    double tmp[4];
    _mm256_store_pd(tmp, v_a);
    a = tmp[0];
    return a;
}}

double run_avx_512(double a, double b, int repeat) {{
    __m512d v_a, v_b;
    v_a = _mm512_set1_pd(a);
    v_b = _mm512_set1_pd(b);

    __m512d v_large = _mm512_set1_pd(LARGE_VALUE);
    __m512d v_a0 = _mm512_set1_pd(a);

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {{
        v_a = _mm512_{op_str};
        v_a = _mm512_add_pd(v_a, v_large);
        v_a = _mm512_sub_pd(v_a, v_large);
        v_a = _mm512_add_pd(v_a, v_a0);
    }}

    double tmp[8];
    _mm512_store_pd(tmp, v_a);
    a = tmp[0];
    return a;
}}

double comp_op(double a, double b) {{
    return {op_comp};
}}
{fmt_func}
}}

'''

    UNARY_FMT_FUNC = \
'''
std::string get_input_normal_subnormal_format(double a, double b) {
    return fpclassify(comp_op(a, b)) + fpclassify(a);
};
'''
    BINARY_FMT_FUNC = \
'''
std::string get_input_normal_subnormal_format(double a, double b) {
    return fpclassify(comp_op(a, b)) + fpclassify(a) + fpclassify(b);
};
'''
    INTEL_OP_DICT = {
        'add': ('a + b',      'add_pd(v_a, v_b)',  BINARY_FMT_FUNC),
        'mul': ('a * b',      'mul_pd(v_a, v_b)',  BINARY_FMT_FUNC),
        'div': ('a / b',      'div_pd(v_a, v_b)',  BINARY_FMT_FUNC),
        'sqrt': ('sqrt(a)',   'sqrt_pd(v_a)',      UNARY_FMT_FUNC),
    }

    for k, (op_comp, op_str, sn_fmt_str) in INTEL_OP_DICT.items():
        with open(os.path.join('include/intel', k + '.h'), 'w') as f:
            f.write(intel_op_fmt_str.format(name=k.upper(), op_comp=op_comp, op_str=op_str, fmt_func=sn_fmt_str))


def arm():
    op_format_str = \
'''
#pragma once
#include <arm_neon.h>
#include "include.h"

namespace op
{{
const std::string NAME="{name}";
const bool has_neon_128 = {has_neon_128};

double run_serial(double a, double b, int repeat) {{
    double a0 = a;

    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {{
        a = {op_comp};
        a += LARGE_VALUE;
        a -= LARGE_VALUE;
        a += a0;
    }}
    return a;
}}

double comp_op(double a, double b) {{
    return {op_comp};
}}
{run_neon_128}
    
{fmt_func}
}};
'''


    neon_128_format_str = \
'''
double run_neon_128(double a, double b, int repeat) {{
    float64x2_t v_a, v_b, v_l, v_a0;
    v_a = vdupq_n_f64(a);
    v_b = vdupq_n_f64(b);
    v_l = vdupq_n_f64(LARGE_VALUE);
    v_a0 = vdupq_n_f64(a);

    // correctness check
    {{
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
            "f{op_name} %[a].2d, %[a].2d, %[b].2d\\n" 
            : [a] "+w" (v_a) 
            : [b] "w" (v_b)
            :
        );
        vst1q_f64(tmp, v_a);
        assert(tmp[0] == comp_op(a, b));
        assert(tmp[1] == comp_op(a, b));
    }}



    #pragma GCC unroll (32)
    for (int i = 0; i < repeat; i++) {{
        asm volatile(
                "f{op_name} %[a].2d, %[a].2d, %[b].2d\\n" 
                "fadd %[a].2d, %[a].2d, %[l].2d\\n" 
                "fsub %[a].2d, %[a].2d, %[l].2d\\n" 
                "fadd %[a].2d, %[a].2d, %[a0].2d\\n" 
                : [a] "+w" (v_a) 
                : [b] "w" (v_b), [l] "w" (v_l), [a0] "w" (v_a0)
                :
            );
    }}

    double tmp[2];
    vst1q_f64(tmp, v_a);
    a = tmp[0];
    return a;
}}
'''

    no_neon_128_str = \
'''
double run_neon_128(double a, double b, int repeat) {
    std::cerr << "Neon 128 not implemented." << std::endl;
    return 0;
}
'''
    UNARY_FMT_FUNC = \
'''
std::string get_input_normal_subnormal_format(double a, double b) {
    return fpclassify(comp_op(a, b)) + fpclassify(a);
};
'''
    BINARY_FMT_FUNC = \
'''
std::string get_input_normal_subnormal_format(double a, double b) {
    return fpclassify(comp_op(a, b)) + fpclassify(a) + fpclassify(b);
};
'''

    def gen_class(name, op_comp, has_neon_128, is_unary, op_name):
        run_neon_128_str = no_neon_128_str
        if has_neon_128:
            run_neon_128_str = neon_128_format_str.format(op_name=op_name)
        fmt_func = UNARY_FMT_FUNC if is_unary else BINARY_FMT_FUNC
        has_neon_128 = 'true' if has_neon_128 else 'false'
        return op_format_str.format(name=name.upper(), op_comp=op_comp, run_neon_128=run_neon_128_str, has_neon_128=has_neon_128, fmt_func=fmt_func)
    

    ARM_OP_DICT = {
        'add': ('a + b', True, False, 'add'),
        'mul': ('a * b', True, False, 'mul'),
        'div': ('a / b', True, False, 'div'),
        'sqrt': ('sqrt(a)', True, True, 'sqrt'),
    }

    for k, (op_comp, has_neon_128, is_unary, op_name) in ARM_OP_DICT.items():
        with open(os.path.join('include/arm', k + '.h'), 'w') as f:
            gen_str = gen_class(k, op_comp, has_neon_128, is_unary, op_name)
            if 'sqrt' in op_name:
                gen_str = gen_str.replace('fsqrt %[a].2d, %[a].2d, %[b].2d', 'fsqrt %[a].2d, %[a].2d')
                gen_str = gen_str.replace('frsqrte %[a].2d, %[a].2d, %[b].2d', 'frsqrte %[a].2d, %[a].2d')
            f.write(gen_str)
        
intel()
arm()