#pragma once

#include "module_loader.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ModuleType {
    MT_LUA,
};

typedef uint8_t(*RawLoadFunction)(const Module *, ModuleOut *);

extern RawLoadFunction _execute_lua_module;
uint8_t _lua_module_info(const char *path, Module *o_module);

#ifdef __cplusplus
}
#endif
