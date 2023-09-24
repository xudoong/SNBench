#!/bin/bash

function intel 
{
    g++ -O3 -I./include -march=native -lpapi main.cpp -DHW_INTEL -DMODE_LATENCY && ./a.out $1 $2
    g++ -O3 -I./include -march=native -lpapi main.cpp -DHW_INTEL -DMODE_THROUGHPUT && ./a.out $1 $2
}

function arm
{
    g++ -O3 -I./include main.cpp -DHW_ARM -DMODE_LATENCY -std=c++17 && ./a.out $1 $2
    g++ -O3 -I./include main.cpp -DHW_ARM -DMODE_THROUGHPUT -std=c++17 && ./a.out $1 $2
}

$1 $2 $3