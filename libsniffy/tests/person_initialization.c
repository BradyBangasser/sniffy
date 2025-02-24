#include <memory.h>

#include "person.h"

int main() {
    Person p0, p1;

    memset(&p1, 0, sizeof(p1));

    if (person_init(&p0)) {
        return 1;
    }

    if (memcmp(&p0, &p1, sizeof(p0))) {
        return 2;
    }

    if (person_destroy(&p0)) {
        return 3;
    }

    return 0;
}
