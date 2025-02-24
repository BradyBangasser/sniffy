#include <memory.h>
#include <mysql/mysql.h>

#include "person.h"
#include "errors.h"

e_err person_init(Person *p) {
    memset(p, 0, sizeof(*p));
    return ERR_OK;
}

e_err person_fetch_by_id(MYSQL *conn, uint8_t id[32], Person *p) {
    static const char *query = "SELECT FirstName, MiddleName, LastName, Suffix, Sex, Race, Height, Weight, Address, PhoneNumber, Note FROM people WHERE ID=?";

    uint64_t fn_l, mn_l, ln_l, sf_l, ad_l, no_l;

    MYSQL_BIND bind[11];
    MYSQL_STMT *stmt;

    memset(bind, 0, sizeof(bind));

    bind[0].buffer = id;
    bind[0].buffer_length = 32;

    if (!(stmt = mysql_stmt_init(conn))) {
        return ERR_MYSQL_STMT_INIT;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        return ERR_MYSQL_STMT_PREPARE;
    }

    if (mysql_stmt_bind_param(stmt, bind)) {
        return ERR_MYSQL_STMT_BIND_PARAM;
    }

    if (mysql_stmt_execute(stmt)) {
        return ERR_MYSQL_STMT_EXE_FAILURE;
    }

    bind[0].buffer = 0;
    bind[0].buffer_length = 0;
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].length = &fn_l;

    bind[1].buffer = 0;
    bind[1].buffer_length = 0;
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].length = &mn_l;

    bind[2].buffer = 0;
    bind[2].buffer_length = 0;
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].length = &ln_l;

    bind[3].buffer = 0;
    bind[3].buffer_length = 0;
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].length = &sf_l;

    bind[4].buffer = &p->sex;
    bind[4].buffer_type = MYSQL_TYPE_TINY;
    bind[5].buffer = &p->race;
    bind[5].buffer_type = MYSQL_TYPE_TINY;
    bind[6].buffer = &p->height;
    bind[6].buffer_type = MYSQL_TYPE_TINY;
    bind[6].is_unsigned = true;
    bind[7].buffer = &p->weight;
    bind[7].buffer_type = MYSQL_TYPE_SHORT;
    bind[7].is_unsigned = true;

    bind[8].buffer = 0;
    bind[8].buffer_type = MYSQL_TYPE_STRING;
    bind[8].buffer_length = 0;
    bind[8].length = &ad_l;

    bind[9].buffer = &p->phone_number;
    bind[9].buffer_type = MYSQL_TYPE_LONG;
    bind[9].is_unsigned = true;

    bind[10].buffer = 0;
    bind[10].buffer_type = MYSQL_TYPE_STRING;
    bind[10].buffer_length = 0;
    bind[10].length = &no_l;

    if (mysql_stmt_bind_result(stmt, bind)) {
        return ERR_MYSQL_STMT_RESULT_BIND_FAILURE;
    }

    if (mysql_stmt_fetch(stmt) == MYSQL_NO_DATA) {
        return ERR_NOT_FOUND;
    }

    p->first_name = calloc(fn_l + 1, sizeof(char));
    if (p->first_name == NULL) {
        return ERR_ALLOC_FAILURE;
    }

    bind[0].buffer = (void *) p->first_name;
    bind[0].buffer_length = fn_l;

    p->middle_name = calloc(mn_l + 1, sizeof(char));
    if (p->middle_name == NULL) {
        return ERR_ALLOC_FAILURE;
    }

    bind[1].buffer = (void *) p->middle_name;
    bind[1].buffer_length = mn_l;

    p->last_name = calloc(ln_l + 1, sizeof(char));
    if (p->last_name == NULL) {
        return ERR_ALLOC_FAILURE;
    }

    bind[2].buffer = (void *) p->last_name;
    bind[2].buffer_length = ln_l;

    p->suffix = calloc(sf_l + 1, sizeof(char));
    if (p->suffix == NULL) {
        return ERR_ALLOC_FAILURE;
    }

    bind[3].buffer = (void *) p->suffix;
    bind[3].buffer_length = sf_l;

    p->address = calloc(ad_l + 1, sizeof(char));
    if (p->address == NULL) {
        return ERR_ALLOC_FAILURE;
    }

    bind[8].buffer = (void *) p->address;
    bind[8].buffer_length = ad_l;

    p->notes = calloc(no_l + 1, sizeof(char));
    if (p->notes == NULL) {
        return ERR_ALLOC_FAILURE;
    }

    bind[10].buffer = (void *) p->notes;
    bind[10].buffer_length = no_l;

    if (
            mysql_stmt_fetch_column(stmt, bind, 0, 0) ||
            mysql_stmt_fetch_column(stmt, bind, 1, 0) ||
            mysql_stmt_fetch_column(stmt, bind, 2, 0) ||
            mysql_stmt_fetch_column(stmt, bind, 3, 0) ||
            mysql_stmt_fetch_column(stmt, bind, 8, 0) ||
            mysql_stmt_fetch_column(stmt, bind, 10, 0)
        ) {
        return ERR_MYSQL_STMT_EXE_FAILURE;
    }

    return ERR_OK;
}
e_err person_fetch_by_detail(MYSQL *conn, Person *p);

e_err person_destroy(Person *p) {
    return ERR_OK;
}
