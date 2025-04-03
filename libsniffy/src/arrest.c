#include <memory.h>

#include "arrest.h"

sniff_e_err sniff_arrest_init(sniff_arrest *arr) {
    memset(arr, 0, sizeof(*arr));
    return ERR_OK;
}

sniff_e_err sniff_arrest_set_charges(sniff_arrest *arr, sniff_charge *c, size_t n_charges) {
    arr->charges = c;
    arr->n_charges = n_charges;
    return ERR_OK;
}

sniff_e_err sniff_arrest_destroy(sniff_arrest *arr) {
    return ERR_OK;
}
