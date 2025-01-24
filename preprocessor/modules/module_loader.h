#pragma once
#include <stdint.h>
#include <linux/limits.h>

#define MAX_MODULES 32

#ifdef __cplusplus
extern "C" {
#endif

enum ModuleType {
    MT_LUA,
};

enum Encoding {
    ENC_JSON,
};

typedef struct {
    const uint8_t module_path[PATH_MAX + 1];
    uint16_t id;
} Module;

typedef struct {
    enum Encoding enc;
    const char *out;
} ModuleOut;

typedef struct {
    const uint8_t facility_address[65];
    const uint8_t facility_name[65];
    const uint8_t state_code[3];
    uint32_t facility_id; // THIS IS SET BY THE PREPROCESSOR
    uint16_t facility_cap;
    uint8_t start_time;
    uint8_t run_interval;
    uint8_t is_incremental;
    enum ModuleType type;
} ModuleMeta;

Module *load_modules(uint16_t *n);
uint8_t fetch_module_meta(const Module *module, ModuleMeta *data);
uint8_t exec_module(uint16_t mid, ModuleOut *out);
uint8_t exec_module_inc(const char *param, uint16_t mid, ModuleOut *out);
ModuleMeta *get_module_meta(uint16_t m);

#ifdef __cplusplus
}
#endif

