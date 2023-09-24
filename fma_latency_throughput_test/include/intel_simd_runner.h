#pragma once 

#include <immintrin.h>
#include "papi.h"
#include "include.h"


class CycleCounter
{
private:
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
    }

    long_long region_end()
    {
        return read() - start_cycle;
    }
};


namespace simd_runner
{
typedef double (*op_func_t)(double, double, double, int);

int choose_repeat(op_func_t func, double a, double b, double c) 
{
    static WallTimer timer;
    const int try_repeat = 5000;
    double foo = 0;
    // warmup
    foo += func(a, b, c, try_repeat);
    // try run
    timer.start();
    foo += func(a, b, c, try_repeat);
    timer.finish();
    double sec = timer.elapsed_ns() * 1e-9 / try_repeat;

    int repeat = MIN_TIMING_SECONDS / sec;
    repeat = std::max(repeat, MIN_REPEAT);
    repeat = std::min(repeat, MAX_REPEAT);
    repeat = ((repeat + REPEAT_UNIT - 1) / REPEAT_UNIT) * REPEAT_UNIT;
    return repeat + (foo > 1e308);
}

double measure_one_simd(op_func_t func, double a, double b, double c, std::vector<double> &cpi_list)
{
    static CycleCounter cycle_counter;
    int repeat = choose_repeat(func, a, b, c);

    cycle_counter.region_begin();
    double rv;
    rv = func(a, b, c, repeat);
    double elapsed_cycles = cycle_counter.region_end();

    double cpi = elapsed_cycles / repeat;
    cpi_list.push_back(cpi);
    return rv;
}

double run_all_simd(double a, double b, double c, std::vector<double> &cpi_list)
{
    double rv = 0;
    rv += measure_one_simd(op::run_serial, a, b, c, cpi_list);
    rv += measure_one_simd(op::run_sse_128, a, b, c, cpi_list);
    rv += measure_one_simd(op::run_avx_256, a, b, c, cpi_list);
    rv += measure_one_simd(op::run_avx_512, a, b, c, cpi_list);
    return rv;
}
} // namespace simd_runner




