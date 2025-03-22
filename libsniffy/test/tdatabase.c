#include <time.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include "tdatabase.h"
#include "test.h"

static char sql_buffer[1024] = {0};

uint8_t test_db_init(MYSQL *conn) {
    time_t t = time(NULL);
    int32_t fd;
    uint32_t i;

    char create_db[64] = "CREATE DATABASE ", buffer[sizeof(t) * 2 + sizeof(TEST_DB_PREFIX) + 3] = {0}, *curs = NULL;

    mysql_init(conn);

    sprintf(buffer, TEST_DB_PREFIX "%lX", t);
    strcat(create_db, buffer);

    assert(mysql_real_connect(conn, TEST_DB_HOST, TEST_DB_UNAME, getenv("TEST_DB_PWORD"), NULL, 0, NULL, 0));
    assert(!mysql_query(conn, create_db));
    assert(!mysql_select_db(conn, buffer));

    assert((fd = open(SQL_DIR "/person.sql", O_RDONLY)) > 0);
    assert(read(fd, sql_buffer, sizeof(sql_buffer)) > 0);
    close(fd);

    curs = sql_buffer;

    i = 0;
    while (sql_buffer[i]) {
        if (sql_buffer[i] == ';') {
            sql_buffer[i] = 0;
            assert(!mysql_query(conn, curs));
            curs = sql_buffer + i + 1;
        }

        i++;
    }

    return 0;
}

uint8_t test_db_destroy(MYSQL *conn) {
    char delete_db[64] = "DROP DATABASE ";
    const char *db = conn->db;

    strcat(delete_db, db);
    if (mysql_query(conn, delete_db)) {
        printf("Error deleting database: %s\n", mysql_error(conn));
        return 1;
    }

    mysql_close(conn);

    memset(conn, 0, sizeof(*conn));

    return 0;
}
