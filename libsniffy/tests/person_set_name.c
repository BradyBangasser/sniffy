#include <assert.h>
#include <string.h>

#include "person.h"

int main() {
    sniff_person p;
    assert(!sniff_person_init(&p));
    assert(!sniff_person_set_name(&p, "Bob", NULL, "Bob Last Name", NULL));

    assert(!strcmp(p.first_name, "Bob"));

    assert(!sniff_person_destroy(&p));

    return 0;
}
