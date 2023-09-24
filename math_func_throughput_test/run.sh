#!/bin/bash

hostname=$(hostname)
if [[ "$hostname" == *csprn1* ]]; then
    device="spr"
elif [[ "$hostname" == camd9654n1* ]]; then
    device="gna"
elif [[ "$hostname" == kp* ]]; then
    device="kp"
else
    device="other"
fi

function sleef_u10
{
    rm result/output.csv
    rm $2
    source ~/scripts/sleef.sh
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_SLEEF_U10 -lsleef -DOP_POW && ./a.out $1 $2
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_SLEEF_U10 -lsleef -DOP_EXP && ./a.out $1 $2
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_SLEEF_U10 -lsleef -DOP_LOG && ./a.out $1 $2
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_SLEEF_U10 -lsleef -DOP_SIN && ./a.out $1 $2
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_SLEEF_U10 -lsleef -DOP_COS && ./a.out $1 $2
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_SLEEF_U10 -lsleef -DOP_TAN && ./a.out $1 $2
}

function sleef_u40
{
    rm result/output.csv
    rm $2
    source ~/scripts/sleef.sh
    # pow is not available
    # exp is not available
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_SLEEF_U35 -lsleef -DOP_LOG && ./a.out $1 $2
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_SLEEF_U35 -lsleef -DOP_SIN && ./a.out $1 $2
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_SLEEF_U35 -lsleef -DOP_COS && ./a.out $1 $2
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_SLEEF_U35 -lsleef -DOP_TAN && ./a.out $1 $2
}

function icc_u10
{
    rm result/output.csv
    rm $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -fimf-max-error=1.0 -DOP_POW && ./a.out $1 $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -fimf-max-error=1.0 -DOP_EXP && ./a.out $1 $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -fimf-max-error=1.0 -DOP_LOG && ./a.out $1 $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -fimf-max-error=1.0 -DOP_SIN && ./a.out $1 $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -fimf-max-error=1.0 -DOP_COS && ./a.out $1 $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -fimf-max-error=1.0 -DOP_TAN && ./a.out $1 $2
}

function icc_u40
{
    rm result/output.csv
    rm $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -fimf-max-error=4.0 -DOP_POW && ./a.out $1 $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -fimf-max-error=4.0 -DOP_EXP && ./a.out $1 $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -fimf-max-error=4.0 -DOP_LOG && ./a.out $1 $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -fimf-max-error=4.0 -DOP_SIN && ./a.out $1 $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -fimf-max-error=4.0 -DOP_COS && ./a.out $1 $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -fimf-max-error=4.0 -DOP_TAN && ./a.out $1 $2
}

function aocc
{
    rm result/output.csv
    rm $2
    source ~/scripts/aocc.sh
    clang++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_AMD -lamdlibm -DOP_POW && ./a.out $1 $2
    clang++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_AMD -lamdlibm -DOP_EXP && ./a.out $1 $2
    clang++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_AMD -lamdlibm -DOP_LOG && ./a.out $1 $2
    clang++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_AMD -lamdlibm -DOP_SIN && ./a.out $1 $2
    clang++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_AMD -lamdlibm -DOP_COS && ./a.out $1 $2
    clang++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DMATH_LIB_AMD -lamdlibm -DOP_TAN && ./a.out $1 $2
}

function gcc_scalar
{
    rm result/output.csv
    rm $2
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DSCALAR_ONLY -DOP_POW && ./a.out $1 $2
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DSCALAR_ONLY -DOP_EXP && ./a.out $1 $2
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DSCALAR_ONLY -DOP_LOG && ./a.out $1 $2
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DSCALAR_ONLY -DOP_SIN && ./a.out $1 $2
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DSCALAR_ONLY -DOP_COS && ./a.out $1 $2
    g++ -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DSCALAR_ONLY -DOP_TAN && ./a.out $1 $2
}

function intel_run_all
{
    if [ -z "$1" ]; then
        local input=input/input.csv
    else
        local input=$1
    fi
    icc_u10 $input result/${device}_icc10.csv
    icc_u40 $input result/${device}_icc40.csv
    sleef_u10 $input result/${device}_sleef10.csv
    sleef_u40 $input result/${device}_sleef40.csv
    aocc $input result/${device}_aocc.csv
    gcc_scalar $input result/${device}_gcc_scalar.csv
}

function arm_gcc_scalar
{
    rm result/output.csv
    rm $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DSCALAR_ONLY -DHW_ARM -DOP_POW && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DSCALAR_ONLY -DHW_ARM -DOP_EXP && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DSCALAR_ONLY -DHW_ARM -DOP_LOG && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DSCALAR_ONLY -DHW_ARM -DOP_SIN && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DSCALAR_ONLY -DHW_ARM -DOP_COS && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DSCALAR_ONLY -DHW_ARM -DOP_TAN && ./a.out $1 $2
}

function arm_sleef_u10
{
    rm result/output.csv
    rm $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_SLEEF_U10 -lsleef -DHW_ARM -DOP_POW && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_SLEEF_U10 -lsleef -DHW_ARM -DOP_EXP && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_SLEEF_U10 -lsleef -DHW_ARM -DOP_LOG && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_SLEEF_U10 -lsleef -DHW_ARM -DOP_SIN && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_SLEEF_U10 -lsleef -DHW_ARM -DOP_COS && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_SLEEF_U10 -lsleef -DHW_ARM -DOP_TAN && ./a.out $1 $2
}

function arm_sleef_u40
{
    rm result/output.csv
    rm $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_SLEEF_U35 -lsleef -DHW_ARM -DOP_LOG && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_SLEEF_U35 -lsleef -DHW_ARM -DOP_SIN && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_SLEEF_U35 -lsleef -DHW_ARM -DOP_COS && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_SLEEF_U35 -lsleef -DHW_ARM -DOP_TAN && ./a.out $1 $2
}

function arm_aml
{
    rm result/output.csv
    rm $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_AML -lmathlib -DHW_ARM -DOP_POW && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_AML -lmathlib -DHW_ARM -DOP_EXP && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_AML -lmathlib -DHW_ARM -DOP_LOG && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_AML -lmathlib -DHW_ARM -DOP_SIN && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DMATH_LIB_AML -lmathlib -DHW_ARM -DOP_COS && ./a.out $1 $2
}

function arm_run_all
{
    if [ -z "$1" ]; then
        local input=input/input.csv
    else
        local input=$1
    fi
    arm_sleef_u10 $input result/${device}_sleef10.csv
    arm_sleef_u40 $input result/${device}_sleef40.csv
    arm_aml $input result/${device}_aml.csv
    arm_gcc_scalar $input result/${device}_gcc_scalar.csv
}

$1 $2 $3