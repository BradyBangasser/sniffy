#include <memory.h>

#include "arrest.h"

e_err arrest_init(Arrest *arr) {
    memset(arr, 0, sizeof(*arr));
    return ERR_OK;
}

e_err arrest_set_charges(Arrest *arr, Charge *c, size_t n_charges) {
    arr->charges = c;
    arr->n_charges = n_charges;
    return ERR_OK;
}

e_err arrest_destroy(Arrest *arr) {
    return ERR_OK;
}
