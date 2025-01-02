#include "sex.h"
#include <string.h>

enum sex str_to_sex(const char *const str) {
    int i = 0;
    while (i < sizeof(MALE)) {
        if (!strcmp(str, MALE[i])) return S_MALE;
        i++;
    }
    return S_FEMALE;
}
