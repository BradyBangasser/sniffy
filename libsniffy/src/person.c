#include <memory.h>

#include "person.h"

e_err person_init(Person *p) {
    memset(p, 0, sizeof(*p));
    return ERR_OK;
}

e_err person_destroy(Person *p) {
    return ERR_OK;
}
