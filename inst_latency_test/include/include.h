#pragma once 

#include <cstdio>
#include <ctime>
#include <unistd.h>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <memory>
#include <map>
#include <cassert>

#include "csv.h"

const double LARGE_VALUE = 1e300;
const double MIN_TIMING_SECONDS = 0.001; 
const int MIN_REPEAT = 10000;
const int MAX_REPEAT = 1000000;

enum { NS_PER_SECOND = 1000000000 };


class WallTimer
{
private:
    struct timespec t_start, t_finish, delta;

    void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
    {
        td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
        td->tv_sec  = t2.tv_sec - t1.tv_sec;
        if (td->tv_sec > 0 && td->tv_nsec < 0)
        {
            td->tv_nsec += NS_PER_SECOND;
            td->tv_sec--;
        }
        else if (td->tv_sec < 0 && td->tv_nsec > 0)
        {
            td->tv_nsec -= NS_PER_SECOND;
            td->tv_sec++;
        }
    }

public:
    WallTimer(){}
    
    void start() {
        clock_gettime(CLOCK_MONOTONIC, &t_start);
    }

    void finish() {
        clock_gettime(CLOCK_MONOTONIC, &t_finish);
        sub_timespec(t_start, t_finish, &delta);
    }

    double elapsed_ns() {
        return delta.tv_sec * 1e9 + delta.tv_nsec;
    }

};



std::string fpclassify(double v) {
    std::string rv;
    switch (std::fpclassify(v))
    {
    case FP_SUBNORMAL:
        rv = "S";
        break;
    case FP_NORMAL:
        rv = "N";
        break;
    case FP_ZERO:
        rv = "O";
        break;
    default:
        std::cout << std::fpclassify(v) << std::endl;
        perror("Error value for classify");
        break;
    }
    return rv;
}
