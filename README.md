# SNBench
A micro benchmark suite that quantifies the performance impact of IEEE-754 subnormal numbers on arithmetic instructions and transcendental functions.

## Features
* Covers all the possible input/output operands configurations in which subnormal numbers appear
* Covers both the scalar instructions and vector instructions
* Covers both the scalar instructions and vector transcendental functions from five math libraries
* support x86 and Arm CPUs
## Directory Layout

* `inst_latency_test`: measure the latency of ADD, MUL, DIV and SQRT instructions on x86 and Arm CPUs.
* `inst_latency_throughput_test`: measure the throughput of of ADD, MUL, DIV and SQRT instructions on x86 and Arm CPUs.
* `fma_latency_throughput_test`: measure the latency and throughput of of the FMA (Fused Multiply Add) instruction on x86 and Arm CPUs.
* `math_func_throughput_test`: measure the throughput of the transcendental math functions

## Dependencies

* PAPI: collect the CPU cyles on x86
* Compilers
  * The GCC Compiler
  * The Intel Compiler: to test the math library SVML
  * The AOCC Compiler: to test the math library AOCL-LibM
* Math libraries:
  * the math library in GLIBC
  * [Intel SVML](https://www.intel.com/content/www/us/en/docs/cpp-compiler/developer-guide-reference/2021-8/intrinsics-for-short-vector-math-library-ops.html)
  * AOCL-LibM: the math library in [AOCL](https://www.amd.com/en/developer/aocl/libm.html)
  * [SLEEF](https://sleef.org/)
  * [Arm Optimized Routines (AML)](https://github.com/ARM-software/optimized-routines)

