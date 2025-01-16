#pragma once

#include "module_loader.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef uint8_t(*RawLoadFunction)(const Module *, ModuleOut *);

extern RawLoadFunction _execute_lua_module;
extern uint8_t _lua_fetch_meta(const Module *, ModuleMeta *);

#ifdef __cplusplus
}
#endif
