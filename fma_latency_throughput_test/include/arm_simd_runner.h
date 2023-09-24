#pragma once 

#include <arm_neon.h>
#include "include.h"


#define _pfh_init_cy                         \
    do {                                    \
        uint64_t _pmu_val0 = 0;             \
        _pmu_val0 = _pmu_val0 | (1 << 31);  \
        asm volatile(                       \
            "mrs x22, pmcr_el0" "\n\t"      \
            "orr x22, x22, #0x7"    "\n\t"  \
            "msr pmcr_el0, x22"     "\n\t"  \
            "msr pmcntenset_el0, %0" "\n\t" \
            :                               \
            : "r" (_pmu_val0)               \
            : "x22"                         \
        );                                  \
    } while(0);


class CycleCounter
{
private:
    long long start_cycle;
    long long pfh_read_cy() {
        long long rv;
        asm volatile("mrs %0, pmccntr_el0"     "\n\t": "=r" (rv)::);
        return rv;
    }
public:
    CycleCounter() {
        _pfh_init_cy
    }
    void region_begin() {
        start_cycle =  pfh_read_cy(); 
    }

    long long region_end()
    {
        return pfh_read_cy() - start_cycle;
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
    double rv = func(a, b, c, repeat);
    double elapsed_cycles = cycle_counter.region_end();

    double cpi = elapsed_cycles / repeat;
    cpi_list.push_back(cpi);
    return rv;
}

double run_all_simd(double a, double b, double c, std::vector<double> &cpi_list)
{
    double rv = 0;
    rv += measure_one_simd(op::run_serial, a, b, c, cpi_list);
    rv += measure_one_simd(op::run_neon_128, a, b, c, cpi_list);
    return rv;
}
} // namespace simd_runner




