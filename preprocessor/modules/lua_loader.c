#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <stdlib.h>

#include "_loader.h"
#include "module_loader.h"

uint8_t _execute_module(const char *fn_name, const char *arg, const Module *module, ModuleOut *o_mod) {
    lua_State *ls = luaL_newstate();
    uint8_t err;
    const char *res = NULL;
    size_t len;

    luaL_openlibs(ls);
    luaopen_base(ls);
    luaopen_io(ls);
    luaopen_string(ls);
    luaopen_package(ls);

    if (luaL_loadfile(ls, (char *) module->module_path) || lua_pcall(ls, 0, 0, 0)) {
        printf("ERROR: %s\n", lua_tostring(ls, -1));
        lua_close(ls);
        return MOE_FILE_LOAD_FAILURE;
    }

    lua_getglobal(ls, fn_name);

    if (arg != NULL) {
        lua_pushstring(ls, arg);
    }

    if (lua_pcall(ls, arg != NULL, 2, 0)) {
        printf("Failed to fetch: %s\n", lua_tostring(ls, -1));
        lua_close(ls);
        return MOE_CALL_FAILURE;
    }

    // TODO ADD TYPE CHECKING HERE
    err = lua_tonumber(ls, -2);

    res = lua_tostring(ls, -1);

    if (res == NULL) return err;

    len = strlen(res);

    o_mod->len = len;
    o_mod->out = malloc(len + 1);
    
    if (o_mod->out == NULL) {
        return MOE_ALLOCATION_FAILED;
    }

    strcpy((char *) o_mod->out, res);

    lua_close(ls);

    return err;
}

RawLoadFunction _execute_lua_module = _execute_module;

uint8_t _lua_fetch_meta(const Module *module, ModuleMeta *data) {
    lua_State *ls = luaL_newstate();
    luaL_openlibs(ls);
    luaopen_base(ls);
    luaopen_io(ls);
    luaopen_string(ls);
    luaopen_package(ls);

    memset((char *) data->state_code, 0, 3);

    if (luaL_loadfile(ls, (char *) module->module_path) || lua_pcall(ls, 0, 0, 0)) {
        printf("ERROR: %s\n", lua_tostring(ls, -1));
        lua_close(ls);
        return 1;
    }

    data->is_incremental = lua_getglobal(ls, "FETCH_INCREMENTAL") != LUA_TNIL;

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

    if (lua_getfield(ls, -1, "state_code") != LUA_TSTRING) {
        printf("In module %s, state_code is wrong type or not defined\n", module->module_path);
        lua_close(ls);
        return 2;
    }

    const char *state_code = lua_tostring(ls, -1);
    memcpy((char *) data->state_code, state_code, 2);

    lua_pop(ls, 1);

    if (lua_getfield(ls, -1, "run_interval") != LUA_TNUMBER) {
        printf("In module %s, facility_cap is wrong type or not defined\n", module->module_path);
        lua_close(ls);
        return 2;
    }

    data->run_interval = lua_tonumber(ls, -1);

    lua_pop(ls, 1);

    if (lua_getfield(ls, -1, "start_time") != LUA_TNUMBER) {
        printf("In module %s, facility_cap is wrong type or not defined\n", module->module_path);
        lua_close(ls);
        return 2;
    }

    data->start_time = lua_tonumber(ls, -1);

    lua_pop(ls, 1);

    if (lua_getfield(ls, -1, "inaccurate_time") == LUA_TBOOLEAN && lua_toboolean(ls, -1)) {
        data->flags |= MO_INACCURATE_TIME;
    }

    lua_pop(ls, 1);

    lua_close(ls);

    data->type = MT_LUA;
    return 0;
}
