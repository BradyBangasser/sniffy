#include "datetimep.h"
uint8_t parse_iso_8601(struct tm *t, const char *str) { return !!strptime(str, "", t); }
