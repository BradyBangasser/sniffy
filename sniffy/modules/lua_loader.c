#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <stdlib.h>

#include "_loader.h"

uint8_t _execute_module(const Module *module, ModuleOut *o_mod) {
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

uint8_t _lua_fetch_meta(const Module *module, ModuleMeta *data) {
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

    if (lua_getglobal(ls, "META_DATA") != LUA_TTABLE) {
        printf("In module %s, META_DATA is wrong type or not defined\n", module->module_path);
        lua_close(ls);
        return 2;
    }

    if (lua_getfield(ls, -1, "facility_name") != LUA_TSTRING) {
        printf("In module %s, facility_name is wrong type or not defined\n", module->module_path);
        lua_close(ls);
        return 2;
    }

    const char *fac_name = lua_tostring(ls, -1);
    strcpy((char *) data->facility_name, fac_name);

    lua_pop(ls, 1);

    if (lua_getfield(ls, -1, "facility_address") != LUA_TSTRING) {
        printf("In module %s, facility_address is wrong type or not defined\n", module->module_path);
        lua_close(ls);
        return 2;
    }

    const char *fac_addr = lua_tostring(ls, -1);
    strcpy((char *) data->facility_address, fac_addr);

    lua_pop(ls, 1);

    if (lua_getfield(ls, -1, "facility_cap") != LUA_TNUMBER) {
        printf("In module %s, facility_cap is wrong type or not defined\n", module->module_path);
        lua_close(ls);
        return 2;
    }

    data->facility_cap = lua_tonumber(ls, -1);

    lua_pop(ls, 1);
    return 0;
}
