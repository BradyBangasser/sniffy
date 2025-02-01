#pragma once
#include <stdint.h>
#include <linux/limits.h>
#include "global.h"

#define MAX_MODULES 32

#ifdef __cplusplus
extern "C" {
#endif

enum ModuleError : uint8_t {
    MOE_OK = 0x0,
    MOE_PARSE_FAILURE = 0x1,
    
    MOE_FILE_LOAD_FAILURE = 0xFC,
    MOE_CALL_FAILURE = 0xFD,
    MOE_ALLOCATION_FAILED = 0xFE,
    MOE_INVALID = 0xFF,
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
    uint32_t len;
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
    uint8_t flags;
    enum RosterType type;
} ModuleMeta;

Module *load_modules(uint16_t *n);
uint8_t fetch_module_meta(const Module *module, ModuleMeta *data);
enum ModuleError exec_module(uint16_t mid, ModuleOut *out);
enum ModuleError exec_module_inc(const char *param, uint16_t mid, ModuleOut *out);
ModuleMeta *get_module_meta(uint16_t m);

#ifdef __cplusplus
}
#endif

