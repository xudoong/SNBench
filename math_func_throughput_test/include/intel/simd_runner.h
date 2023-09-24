#pragma once 

#include <immintrin.h>
#include "papi.h"
#include "include.h"


namespace simd_runner
{
typedef double (*op_func_t)(const double *, const double *, int, int);

class CycleCounter
{
private:
    WallTimer timer;
    int event_set=PAPI_NULL;
    long_long values[1];
    long_long start_cycle;

    int retval;

private:
    long_long read() 
    {
        /* Read the counting events in the Event Set */
        if (PAPI_read(event_set, values) != PAPI_OK) {
            fprintf(stderr, "PAPI_read error!\n");
            exit(1);
        }
        return values[0];
    }

public:
    CycleCounter()
    {
        /* Initialize the PAPI library */
        retval = PAPI_library_init(PAPI_VER_CURRENT);
        if (retval != PAPI_VER_CURRENT) {
            fprintf(stderr, "PAPI library init error!\n");
            exit(1);
        }

        /* Create the Event Set */
        if (PAPI_create_eventset(&event_set) != PAPI_OK) {
            fprintf(stderr, "PAPI_create_eventset error!\n");
            exit(1);
        }

        if (PAPI_add_event(event_set, PAPI_TOT_CYC) != PAPI_OK) {
            fprintf(stderr, "PAPI_add_event error!\n");
            exit(1);
        }

        /* Start counting events in the Event Set */
        if (PAPI_start(event_set) != PAPI_OK) {
            fprintf(stderr, "PAPI_start error!\n");
            exit(1);
        }
    }

    ~CycleCounter() 
    {
        if (PAPI_stop(event_set, values) != PAPI_OK) {
            fprintf(stderr, "PAPI_stop error!\n");
            exit(1);
        }
    }
    void region_begin() 
    {
        start_cycle = read();
        timer.start();
    }

    long_long region_end()
    {
        timer.finish();
        return read() - start_cycle;
    }
    double elapsed_ns() {
        return timer.elapsed_ns();
    }
};

int choose_repeat(op_func_t func, const double *a, const double *b, int n) 
{
    static WallTimer timer;
    const int try_repeat = 3;
    double tmp;
    // warmup
    // seperately call run_avx_512 from other SIMD funcs because of the annoying (avx-512, Intel SVML, function pointer) bug in AMD
#ifndef SCALAR_ONLY
    if (func == op::run_avx_512) {
        tmp = op::run_avx_512(a, b, n, try_repeat);
    } else {
        tmp = func(a, b, n, try_repeat);
    }
#else 
    tmp = func(a, b, n, try_repeat);
#endif
    // try run
    timer.start();
#ifndef SCALAR_ONLY
    if (func == op::run_avx_512) {
        op::run_avx_512(a, b, n, try_repeat);
    } else {
        func(a, b, n, try_repeat);
    }
#else 
    tmp = func(a, b, n, try_repeat);
#endif
    timer.finish();
    double sec = timer.elapsed_ns() * 1e-9 / try_repeat;

    int repeat = MIN_TIMING_SECONDS / sec;
    repeat = std::max(repeat, MIN_REPEAT);
    repeat = std::min(repeat, MAX_REPEAT);
    return repeat + (tmp > 1e308);
}

double measure_one_simd(op_func_t func, const double *a, const double *b, int n, std::vector<double> &cpi_list)
{
    // can not be static because of the AMD Intel SVML AVX-512 bug
    CycleCounter cycle_counter;
    int repeat = choose_repeat(func, a, b, n);
    cycle_counter.region_begin();
    double rv;
#ifndef SCALAR_ONLY
    if (func == op::run_avx_512) {
        rv = op::run_avx_512(a, b, n, repeat);
    } else {
        rv = func(a, b, n, repeat);
    }
#else 
    rv = func(a, b, n, repeat);
#endif
    // 1. collect nano seconds
    // cycle_counter.region_end();
    // double elapsed_time = cycle_counter.elapsed_ns();
    // double cpi = elapsed_time / repeat / n;
    // 2. collect cycle
    double cpi = double(cycle_counter.region_end()) / repeat / n;
    cpi_list.push_back(cpi);
    return rv;
}

double run_all_simd(const double *a, const double *b, int n, std::vector<double> &cpi_list)
{
    double rv = 0;
    rv += measure_one_simd(op::run_serial, a, b, n, cpi_list);
#ifndef SCALAR_ONLY
    rv += measure_one_simd(op::run_sse_128, a, b, n, cpi_list);
    rv += measure_one_simd(op::run_avx_256, a, b, n, cpi_list);
    rv += measure_one_simd(op::run_avx_512, a, b, n, cpi_list);
#endif
    return rv;
}
} // namespace simd_runner




