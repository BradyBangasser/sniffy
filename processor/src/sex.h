#pragma once

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif
    static const char *MALE[] = { "m", "male" };
    enum sex str_to_sex(const char *const str);
#ifdef __cplusplus
}
#endif

