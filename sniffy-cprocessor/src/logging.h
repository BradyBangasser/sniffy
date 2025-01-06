#pragma once

#define M_DEBUG

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <time.h>
#include <memory.h>
#include <errno.h>
#include <inttypes.h>


#ifdef __cplusplus
extern "C" {
#endif

#define TIMING_CLOCK CLOCK_MONOTONIC

#define COLOR_RESET "\x1b[0m"

#ifndef ERROR_COLOR
#define ERROR_COLOR "\x1b[31m"
#endif

#ifndef DEBUG_COLOR
#define DEBUG_COLOR "\x1b[36m"
#endif

#ifndef SUCCESS_COLOR
#define SUCCESS_COLOR "\x1b[32m"
#endif

#ifndef WARNING_COLOR
#define WARNING_COLOR "\x1b[33m"
#endif

#ifndef INFO_COLOR
#define INFO_COLOR "\x1b[34m"
#endif

#ifndef MAGENTIA
#define MAGENTIA "\x1b[35m"
#endif

extern uint32_t nerrs;
extern uint32_t nwarns;
extern uint64_t unfreed_allocs;

static struct timespec ts;

#define ERRORF(f, ...) fprintf(stderr, ERROR_COLOR "[ERROR]   " f COLOR_RESET, __VA_ARGS__); ++nerrs
#define ERROR(f) fprintf(stderr, ERROR_COLOR "[ERROR]   " f COLOR_RESET); ++nerrs
#define WARNF(f, ...) fprintf(stderr, WARNING_COLOR "[WARNING] " f COLOR_RESET, __VA_ARGS__); ++nwarns
#define WARN(f) fprintf(stderr, WARNING_COLOR "[WARNING] " f COLOR_RESET); ++nwarns
#define INFOF(f, ...) printf(INFO_COLOR "[INFO]    " f COLOR_RESET, __VA_ARGS__)
#define INFO(f) printf(INFO_COLOR "[INFO]    " f COLOR_RESET)
#define SUCCESS(f) printf(SUCCESS_COLOR "[SUCCESS] " f COLOR_RESET)
#define SUCCESSF(f, ...) printf(SUCCESS_COLOR "[SUCCESS] " f COLOR_RESET, __VA_ARGS__)

#ifdef M_DEBUG
#define DEBUG(f) printf(DEBUG_COLOR "[DEBUG]   " f COLOR_RESET)
#define DEBUGF(f, ...) printf(DEBUG_COLOR "[DEBUG]   " f COLOR_RESET, __VA_ARGS__)

#else
#define DEBUG
#define DEBUGF
#endif

static inline void start_time() {
    clock_gettime(TIMING_CLOCK, &ts);
}

static inline void stop_time_print_data() {
    struct timespec ets;

    if (clock_gettime(TIMING_CLOCK, &ets) != 0) {
        ERRORF("Failed to start clock, errno %d\n", errno);
        return;
    }

    uint32_t tsec = ets.tv_sec - ts.tv_sec;
    uint32_t tnsec = ets.tv_nsec -= ts.tv_nsec;

    uint32_t hours = tsec / 3600,
             minutes = (tsec % 3600) / 60 % 60;
    tsec %= 60;

    printf(MAGENTIA "\n | -----Execution Stats-----\n | Run Time: ");
    if (hours) printf("%dh ", hours);
    if (minutes) printf("%dm ", minutes);
    if (tsec > 0) printf("%ds ", tsec);
    printf("%.4f ms\n", tnsec / 1.0e7);

    printf(" | Issues: ");
    if (nerrs > 0) printf(ERROR_COLOR);
    printf("%d" MAGENTIA " errors; ", nerrs);
    if (nwarns > 0) printf(WARNING_COLOR);
    printf("%d" MAGENTIA " warnings\n | \n\n", nwarns);
    
    printf(COLOR_RESET);
}

static inline struct timespec stop_time() {
    struct timespec ets;
    if (clock_gettime(TIMING_CLOCK, &ets) != 0) {
        memset(&ets, 0, sizeof(ets));
        ERRORF("Failed to start clock, errno %d\n", errno);
        return ets;
    }

    ets.tv_sec -= ts.tv_sec;
    ets.tv_nsec -= ts.tv_nsec;
    return ets;
}

#pragma GCC poison printf fprintf

#ifdef __cplusplus
}
#endif
