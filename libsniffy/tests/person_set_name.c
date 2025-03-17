#include <assert.h>
#include <string.h>

#include "person.h"

int main() {
    Person p;
    assert(!person_init(&p));
    assert(!person_set_name(&p, "Bob", NULL, "Bob Last Name", NULL));

    assert(!strcmp(p.first_name, "Bob"));

    assert(!person_destroy(&p));

    return 0;
}
