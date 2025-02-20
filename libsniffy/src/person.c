#include <memory.h>

#include "person.h"
#include "errors.h"

e_err person_init(Person *p) {
    memset(p, 0, sizeof(*p));
    return ERR_OK;
}

e_err person_fetch_by_id(MYSQL *conn, uint8_t id[32], Person *p) {
    static const char *query = "SELECT ";
    MYSQL_BIND bind[8];
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
}
e_err person_fetch_by_detail(MYSQL *conn, Person *p);

e_err person_destroy(Person *p) {
    return ERR_OK;
}
