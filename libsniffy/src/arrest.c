#include <memory.h>

#include "arrest.h"

e_err arrest_init(Arrest *arr) {
    memset(arr, 0, sizeof(*arr));
    return ERR_OK;
}

e_err arrest_destroy(Arrest *arr) {
    return ERR_OK;
}
