#include <assert.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include <stdio.h>

#include "tdatabase.h"
#include "person.h"

static MYSQL c;
static uint8_t id[32] = {0};

void onxt() {
    test_db_destroy(&c);
}

int main() {
    sniff_person p;
    sniff_e_err r;

    assert(!test_db_init(&c));
    atexit(onxt);

    sniff_person_init(&p);

    sniff_person_set_name(&p, "joe", NULL, "Mama", NULL);
    sniff_person_set_birth_year(&p, 120);

    r = sniff_person_upsert(&c, &p);
    if (r) {
        printf("Failed to upsert, error: %s\n", sniff_err_to_str(r));
        return 1;
    }

    sniff_person_destroy(&p);
    r = sniff_person_fetch_by_id(&c, id, &p);

    return r;
}
