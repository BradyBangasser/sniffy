#include <assert.h>
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>

#include "tdatabase.h"
#include "person.h"

static MYSQL c;
static uint8_t id[32] = {0};

void onxt() {
    test_db_destroy(&c);
}

int main() {
    Person p;
    e_err r;

    assert(!test_db_init(&c));
    atexit(onxt);

    person_init(&p);

    person_set_name(&p, "joe", NULL, "Mama", NULL);
    person_set_birth_year(&p, 120);

    r = person_upsert(&c, &p);
    if (r) {
        printf("Failed to upsert, error: %s\n", err_codes[r]);
        return 1;
    }

    person_destroy(&p);
    r = person_fetch_by_id(&c, id, &p);
    printf("%s\n", err_codes[r]);

    return ERR_NOT_IMPLEMENTED;
}
