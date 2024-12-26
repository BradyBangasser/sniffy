#include "database.h"
#include <stdlib.h>
#include <mysql/mysql_com.h>
#include <stdio.h>

#define DEFAULT_DB_USERNAME "sniffy"
#define DB "inmate_server"

static MYSQL conn;
static uint8_t not_connected = 1;

bool connect() {
    const char *db_username = getenv("db_username");
    if (db_username == NULL) db_username = DEFAULT_DB_USERNAME;

    mysql_init(&conn);
    if (!mysql_real_connect(&conn, NULL, db_username, getenv("db_pass"), DB, 0, NULL, 0)) {
        printf("Failed to connect to database, error: %s\n", mysql_error(&conn));
        return false;
    }

    not_connected = 0;

    return true;
}

MYSQL *get_connection() { return not_connected ? NULL : &conn; }
