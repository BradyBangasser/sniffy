#pragma once

#include <inttypes.h>

#include "arrest.h"

enum E_ChargeImpFlags {
    ECIF_ID_SET = 0x1,
    ECIF_FULL_ARR_SET = 0x2,
};

typedef struct Charge {
    uint64_t id;
    union {
        uint64_t aid;
        Arrest *a;
    } arr;
    
    char sid[32];
    char *docket;

    uint32_t _iflags;
} Charge;
