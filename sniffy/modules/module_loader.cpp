#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <functional>
#include <unistd.h>

#include "module_loader.h"
#include "_loader.h"

const char *ignored_files[] = { "README.md", "module_loader.cpp", "module_loader.h", "lua_loader.c" };

typedef std::function<std::remove_pointer_t<RawLoadFunction>> ModuleCall;

Module modules[MAX_MODULES] = {};
ModuleCall cmodules[MAX_MODULES] = {0};
ModuleMeta mdata[MAX_MODULES] = {0};
static uint16_t n_mods = 0;

extern "C" Module *load_modules(uint16_t *n) {
    if (n_mods > 0) {
        *n = n_mods;
        return modules;
    }

    DIR *d;
    int i;
    struct dirent *dir;
    chdir("modules");
    d = opendir(".");

    if (d != NULL) {
        while ((dir = readdir(d)) != NULL) {
            switch (dir->d_name[0]) {
                case 0x5F:
                case 0x2E:
                case 0x21:
                    continue;
            }

            for (i = 0; i < sizeof(ignored_files) / sizeof(char *); i++) {
                if (strcmp(ignored_files[i], dir->d_name) == 0) {
                    i = -1;
                    break;
                }
            }

            if (i == -1) {
                continue;
            }

            realpath(dir->d_name, (char *) modules[n_mods].module_path);

            if (modules[n_mods].module_path[0] == 0x0) {
                printf("Error getting module path\n");
                continue;
            }

            if (fetch_module_meta(modules + n_mods, mdata + n_mods)) {
                printf("Error fetching metadata for module '%s'\n", modules[n_mods].module_path);
            }

            cmodules[n_mods] = std::bind_front(_execute_lua_module);
            modules[n_mods].id = n_mods;

            printf("Registering module SM-%08X (%s)\n", n_mods, mdata[n_mods].facility_name);

            n_mods++;
        }

        *n = n_mods;

        if (n_mods == 0) {
            printf("Warning: 0 modules detected\n");
        }

        return modules;
    } else {
        printf("Failed to open directory\n");
        return 0;
    }
}

extern "C" uint8_t exec_module(uint16_t mid, ModuleOut *out) {
    if (cmodules[mid] == NULL || modules[mid].module_path[0] == 0x0) {
        return 1;
    }

    uint8_t res = cmodules[mid](&modules[mid], out);

    return -1;
}

const char *get_ext(const char *path, int16_t length) {
    int16_t i;
    for (i = length - 1; i >= 0 && path[i] != '.'; i--);
    return i == -1 ? NULL : path + i;
}
extern "C" uint8_t fetch_module_meta(const Module *module, ModuleMeta *data) {
    uint16_t module_path_len = strlen((char *) module->module_path);
    const char *ext = get_ext((char *) module->module_path, module_path_len);

    if (ext == NULL) {
        return 1;
    }

    uint16_t extlen = module_path_len - (ext - (char *) module->module_path);

    if (strncmp(".lua", ext, extlen) == 0) {
        return _lua_fetch_meta(module, data);
    } else {
        printf("Unknown Module Type '%s' (%s)\n", ext, module->module_path);
        return 2;
    }

    return 0;
}

extern "C" ModuleMeta *get_module_meta(uint16_t m) {
    if (m >= n_mods) return NULL;
    return mdata + m;
}
