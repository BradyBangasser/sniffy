#include <stdio.h>
#include "modules/module_loader.h"

int main() {
    uint16_t n = 0;
    load_modules(&n);
    ModuleOut m;
    exec_module(0, &m);

    printf("%s\n", m.out);
}
