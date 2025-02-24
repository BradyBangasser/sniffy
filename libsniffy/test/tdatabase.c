#include <time.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#include "tdatabase.h"

uint8_t test_db_init(MYSQL *conn) {
    char create_db[64] = "CREATE DATABASE ";
    time_t t = time(NULL);
    MYSQL_BIND bind;
    char buffer[sizeof(t) * 2 + sizeof(TEST_DB_PREFIX) + 3] = {0};

    mysql_init(conn);

    sprintf(buffer, TEST_DB_PREFIX "-%lX", t);
    strcat(create_db, buffer);

    assert(mysql_real_connect(conn, TEST_DB_HOST, TEST_DB_UNAME, getenv("TEST_DB_PWORD"), NULL, 0, NULL, 0));
    assert(!mysql_query(conn, create_db));
    assert(!mysql_select_db(conn, buffer));

    return 0;
}

uint8_t test_db_destroy(MYSQL *conn) {
    char delete_db[64] = "CREATE DATABASE ";
    const char *db = conn->db;

    strcat(delete_db, db);
    assert(!mysql_select_db(conn, NULL));
    assert(!mysql_query(conn, delete_db));

    mysql_close(conn);

    memset(conn, 0, sizeof(*conn));

    return 0;
}
