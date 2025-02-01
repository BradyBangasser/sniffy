#include "race.h"
#include "logging.h"

#include <string.h>

enum race str_to_race(const char * const str) {
    int i = 0;

    while (i < sizeof(WHITE) / sizeof(char *)) {
        if (!strcmp(str, WHITE[i])) return SR_WHITE;
        i++;
    }

    i = 0;
    while (i < sizeof(BLACK) / sizeof(char *)) {
        if (!strcmp(str, BLACK[i])) return SR_BLACK;
        i++;
    }
    
    WARNF("'%s' didn't match any races\n", str);

    return SR_UNKNOWN;
}
