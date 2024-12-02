#pragma once
#include <stdint.h>
#include <linux/limits.h>

#define MAX_MODULES 32

#ifdef __cplusplus
extern "C" {
#endif

enum Encoding {
    ENC_JSON,
};

typedef struct {
    const uint8_t module_path[PATH_MAX + 1];
    const uint8_t display_name[64];
    uint8_t start_time;
    uint8_t run_interval;
    uint16_t id;
} Module;

typedef struct {
    enum Encoding enc;
    const char *out;
} ModuleOut;

Module *load_modules(uint16_t *n);
uint8_t exec_module(uint16_t mid, ModuleOut *out);

#ifdef __cplusplus
}
#endif

