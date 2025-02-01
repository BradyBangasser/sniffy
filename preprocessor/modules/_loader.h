#pragma once

#include "module_loader.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum ModuleError (*RawLoadFunction)(const char *fn_name, const char *param, const Module *, ModuleOut *);

extern RawLoadFunction _execute_lua_module;
extern enum ModuleError _lua_fetch_meta(const Module *, ModuleMeta *);

#ifdef __cplusplus
}
#endif
