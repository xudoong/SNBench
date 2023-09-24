#!/bin/bash

function intel
{
    rm result/output.csv
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DOP_ADD && ./a.out $1 $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DOP_MUL && ./a.out $1 $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DOP_DIV && ./a.out $1 $2
    icpx -fp-model=precise -O3 -march=native main.cpp -lpapi -DHW_INTEL -I./include -std=c++17 -DOP_SQRT && ./a.out $1 $2
}

function arm
{
    rm result/output.csv
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DHW_ARM -DOP_ADD && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DHW_ARM -DOP_MUL && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DHW_ARM -DOP_DIV && ./a.out $1 $2
    g++ -O3 main.cpp -I./include -I. -std=c++17 -DHW_ARM -DOP_SQRT && ./a.out $1 $2
}

$1 $2 $3