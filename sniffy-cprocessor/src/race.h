#pragma once

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif
    static const char *WHITE[] = { "white", "w", "caucasian" };
    static const char *BLACK[] = { "black", "b", "african american", "aa" };
    
    enum race str_to_race(const char * const str);

#ifdef __cplusplus
}
#endif
