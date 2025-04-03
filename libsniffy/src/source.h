#ifndef H_SNIFF_SOURCE
#define H_SNIFF_SOURCE

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        char id[32];
        char *name;
        uint8_t n_l;
    } sniff_src;

#ifdef __cplusplus
}
#endif
#endif
