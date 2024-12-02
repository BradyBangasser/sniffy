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
            if (dir->d_name[0] == 0x5F || dir->d_name[0] == 0x2E || dir->d_name[0] == 0x21) {
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

            strcpy((char *) modules[n_mods].display_name, dir->d_name);
            modules[n_mods].start_time = 0;
            modules[n_mods].run_interval = 30;
            cmodules[n_mods] = std::bind_front(_execute_lua_module);
            n_mods++;

            printf("Registering module %d (%s)\n", n_mods, dir->d_name);
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
