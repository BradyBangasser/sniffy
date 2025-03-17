#include <assert.h>
#include <mysql/mysql.h>
#include <stdio.h>

#include "tdatabase.h"
#include "person.h"

int main() {
    MYSQL c;
    assert(!test_db_init(&c));
    assert(!test_db_destroy(&c));

    return ERR_NOT_IMPLEMENTED;
}
