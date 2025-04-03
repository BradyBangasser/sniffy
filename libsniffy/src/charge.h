#pragma once

#include <inttypes.h>

#include "arrest.h"

enum sniff_e_ciflags {
    ECIF_ID_SET = 0x1,
    ECIF_FULL_ARR_SET = 0x2,
};

typedef struct sniff_charge {
    uint64_t id;
    union {
        uint64_t aid;
        sniff_arrest *a;
    } arr;
    
    char sid[32];
    char *docket;

    uint32_t _iflags;
} sniff_charge;
