import os

def intel():
    intel_op_fmt_str = \
'''
#pragma once
#include <immintrin.h>
#include "papi.h"
#include "include.h"

#if defined(MATH_LIB_AMD)
#define AMD_LIBM_VEC_EXPERIMENTAL
#include "amdlibm.h"
#include "amdlibm_vec.h"
#elif defined(MATH_LIB_SLEEF_U10) || defined(MATH_LIB_SLEEF_U35)
#include "sleef.h"
#endif

namespace op
{{
const std::string NAME="{name_upper}";
alignas(64) double out[MAX_N];

double run_serial(const double *a, const double *b, int n, int repeat) {{
    for (int rp = 0; rp < repeat; rp++) {{
        for (int i = 0; i < n; i++) {{
#if defined(MATH_LIB_AMD)
            out[i] = amd_{name}(a[i]);
#elif defined(MATH_LIB_SLEEF_U10)
            out[i] = Sleef_{name}_u10(a[i]);
#elif defined(MATH_LIB_SLEEF_U35)
            out[i] = Sleef_{name}_u35(a[i]);
#else 
            out[i] = {name}(a[i]);
#endif
        }}
    }}
    return out[0];
}}


#ifndef SCALAR_ONLY
double run_sse_128(const double *a, const double *b, int n, int repeat) {{
    for (int rp = 0; rp < repeat; rp++) {{
        for (int i = 0; i < n; i+=2) {{
            __m128d v_a = _mm_load_pd(a + i);
#if defined(MATH_LIB_AMD)
            v_a = amd_vrd2_{name}(v_a);
#elif defined(MATH_LIB_SLEEF_U10)
            v_a = Sleef_{name}d2_u10(v_a);
#elif defined(MATH_LIB_SLEEF_U35)
            v_a = Sleef_{name}d2_u35(v_a);
#else 
            v_a = _mm_{name}_pd(v_a);
#endif
            _mm_store_pd(out + i, v_a);
        }}
    }}
    return out[0];
}}

double run_avx_256(const double *a, const double *b, int n, int repeat) {{
    for (int rp = 0; rp < repeat; rp++) {{
        for (int i = 0; i < n; i+=4) {{
            __m256d v_a = _mm256_load_pd(a + i);
#if defined(MATH_LIB_AMD)
            v_a = amd_vrd4_{name}(v_a);
#elif defined(MATH_LIB_SLEEF_U10)
            v_a = Sleef_{name}d4_u10(v_a);
#elif defined(MATH_LIB_SLEEF_U35)
            v_a = Sleef_{name}d4_u35(v_a);
#else 
            v_a = _mm256_{name}_pd(v_a);
#endif
            _mm256_store_pd(out + i, v_a);
        }}
    }}
    return out[0];
}}

double run_avx_512(const double *a, const double *b, int n, int repeat) {{
    for (int rp = 0; rp < repeat; rp++) {{
        for (int i = 0; i < n; i+=8) {{
            __m512d v_a = _mm512_load_pd(a + i);
#if defined(MATH_LIB_AMD)
            v_a = amd_vrd8_{name}(v_a);
#elif defined(MATH_LIB_SLEEF_U10)
            v_a = Sleef_{name}d8_u10(v_a);
#elif defined(MATH_LIB_SLEEF_U35)
            v_a = Sleef_{name}d8_u35(v_a);
#else 
            v_a = _mm512_{name}_pd(v_a);
#endif
            _mm512_store_pd(out + i, v_a);
        }}
    }}
    return out[0];
}}
#endif

double comp_op(double a, double b) {{
#if defined(MATH_LIB_AMD)
    a = amd_{name}(a);
#elif defined(MATH_LIB_SLEEF_U10)
    a = Sleef_{name}_u10(a);
#elif defined(MATH_LIB_SLEEF_U35)
    a = Sleef_{name}_u35(a);
#else 
    a = {name}(a);
#endif
    return a;
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
        'pow': (BINARY_FMT_FUNC),
        'exp': (UNARY_FMT_FUNC),
        'log': (UNARY_FMT_FUNC),
        'sin': (UNARY_FMT_FUNC),
        'cos': (UNARY_FMT_FUNC),
        'tan': (UNARY_FMT_FUNC),
    }

    for name, (sn_fmt_str) in INTEL_OP_DICT.items():
        with open(os.path.join('include/intel', name + '.h'), 'w') as f:
            text_to_write = intel_op_fmt_str.format(name_upper=name.upper(), name=name, fmt_func=sn_fmt_str)
            # process pow
            if name == 'pow':
                text_to_write = text_to_write.replace('pow(a);', 'pow(a, b);')
                text_to_write = text_to_write.replace('u10(a);', 'u10(a, b);')
                text_to_write = text_to_write.replace('pow(a[i]);', 'pow(a[i], b[i]);')
                text_to_write = text_to_write.replace('u10(a[i]);', 'u10(a[i], b[i]);')
                text_to_write = text_to_write.replace('u35(a[i]);', 'u35(a[i], b[i]);')
                text_to_write = text_to_write.replace('u10(v_a);', 'u10(v_a, v_b);')
                text_to_write = text_to_write.replace('u35(v_a);', 'u35(v_a, v_b);')
                text_to_write = text_to_write.replace('pow_pd(v_a);', 'pow_pd(v_a, v_b);')
                text_to_write = text_to_write.replace('pow(v_a);', 'pow(v_a, v_b);')
                text_to_write = text_to_write.replace('__m128d v_a = _mm_load_pd(a + i);', '__m128d v_a = _mm_load_pd(a + i);\n\t\t\t__m128d v_b = _mm_load_pd(b + i);')
                text_to_write = text_to_write.replace('__m256d v_a = _mm256_load_pd(a + i);', '__m256d v_a = _mm256_load_pd(a + i);\n\t\t\t__m256d v_b = _mm256_load_pd(b + i);')
                text_to_write = text_to_write.replace('__m512d v_a = _mm512_load_pd(a + i);', '__m512d v_a = _mm512_load_pd(a + i);\n\t\t\t__m512d v_b = _mm512_load_pd(b + i);')
            f.write(text_to_write)

        


def arm():
    op_format_str = \
'''

#pragma once
#include <arm_neon.h>
#include "include.h"
#if defined(MATH_LIB_AML)
extern "C" {{
#include "mathlib.h"
}}
#elif defined(MATH_LIB_SLEEF_U10) || defined(MATH_LIB_SLEEF_U35)
#include "sleef.h"
#endif

namespace op
{{
const std::string NAME="{name_upper}";
alignas(64) double out[MAX_N];

double run_serial(const double *a, const double *b, int n, int repeat) {{
    for (int rp = 0; rp < repeat; rp++) {{
        for (int i = 0; i < n; i++) {{
#if defined(MATH_LIB_AML)
            out[i] = __s_{name}(a[i]);
#elif defined(MATH_LIB_SLEEF_U10)
            out[i] = Sleef_{name}_u10(a[i]);
#elif defined(MATH_LIB_SLEEF_U35)
            out[i] = Sleef_{name}_u35(a[i]);
#else 
            out[i] = {name}(a[i]);
#endif
        }}
    }}
    return out[0];
}}


#ifndef SCALAR_ONLY
double run_neon_128(const double *a, const double *b, int n, int repeat) {{
    for (int rp = 0; rp < repeat; rp++) {{
        for (int i = 0; i < n; i+=2) {{
            float64x2_t v_a = vld1q_f64(a + i);
#if defined(MATH_LIB_AML)
            v_a = __v_{name}(v_a);
#elif defined(MATH_LIB_SLEEF_U10)
            v_a = Sleef_{name}d2_u10(v_a);
#elif defined(MATH_LIB_SLEEF_U35)
            v_a = Sleef_{name}d2_u35(v_a);
#endif
            vst1q_f64(out + i, v_a);
        }}
    }}
    return out[0];
}}
#endif

double comp_op(double a, double b) {{
#if defined(MATH_LIB_AML)
            a = __s_{name}(a);
#elif defined(MATH_LIB_SLEEF_U10)
            a = Sleef_{name}_u10(a);
#elif defined(MATH_LIB_SLEEF_U35)
            a = Sleef_{name}_u35(a);
#else 
            a = {name}(a);
#endif
    return a;
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

    ARM_OP_DICT = {
        'pow': BINARY_FMT_FUNC,
        'exp': UNARY_FMT_FUNC,
        'log': UNARY_FMT_FUNC,
        'sin': UNARY_FMT_FUNC,
        'cos': UNARY_FMT_FUNC,
        'tan': UNARY_FMT_FUNC,
    }

    for k, fmt_func in ARM_OP_DICT.items():
        with open(os.path.join('include/arm', k + '.h'), 'w') as f:
            text_to_write = op_format_str.format(name_upper=k.upper(), name=k, fmt_func=fmt_func)
            if k == 'pow':
                text_to_write = text_to_write.replace('pow(a);', 'pow(a, b);')
                text_to_write = text_to_write.replace('u10(a);', 'u10(a, b);')
                text_to_write = text_to_write.replace('u35(a);', 'u35(a, b);')
                text_to_write = text_to_write.replace('pow(a[i]);', 'pow(a[i], b[i]);')
                text_to_write = text_to_write.replace('u10(a[i]);', 'u10(a[i], b[i]);')
                text_to_write = text_to_write.replace('u35(a[i]);', 'u35(a[i], b[i]);')
                text_to_write = text_to_write.replace('pow(v_a);', 'pow(v_a, v_b);')
                text_to_write = text_to_write.replace('u10(v_a);', 'u10(v_a, v_b);')
                text_to_write = text_to_write.replace('u35(v_a);', 'u35(v_a, v_b);')
                text_to_write = text_to_write.replace('float64x2_t v_a = vld1q_f64(a + i);', 'float64x2_t v_a = vld1q_f64(a + i);\n\t\t\tfloat64x2_t v_b = vld1q_f64(b + i);')
            f.write(text_to_write)
        
intel()
arm()