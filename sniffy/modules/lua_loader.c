#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <stdlib.h>

#include "_loader.h"

uint8_t _lua_module_info(const char *path, Module *o_module) {
    o_module->start_time = 0;
    o_module->run_interval = 30;
    return 0;
}

uint8_t _execute_module(const Module *module, ModuleOut *o_mod) {
    printf("Executing module %s\n", module->display_name);
    lua_State *ls = luaL_newstate();
    luaL_openlibs(ls);
    luaopen_base(ls);
    luaopen_io(ls);
    luaopen_string(ls);
    luaopen_package(ls);

    if (luaL_loadfile(ls, (char *) module->module_path) || lua_pcall(ls, 0, 0, 0)) {
        printf("ERROR: %s\n", lua_tostring(ls, -1));
        lua_close(ls);
        return 1;
    }

    lua_getglobal(ls, "FETCH");

    if (lua_pcall(ls, 0, 1, 0)) {
        printf("Failed to fetch: %s\n", lua_tostring(ls, -1));
        lua_close(ls);
        return -1;
    }

    const char *res = lua_tostring(ls, -1);
    o_mod->out = malloc(strlen(res) + 1);
    strcpy((char *) o_mod->out, res);

    lua_close(ls);

    return -1;
}

RawLoadFunction _execute_lua_module = _execute_module;
