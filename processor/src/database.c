#include "database.h"
#include <stdlib.h>
#include <mysql/mysql_com.h>

#include "logging.h"

#define DEFAULT_DB_USERNAME "sniffy"
#define DB "sniffy"

static MYSQL conn;
static uint8_t not_connected = 1;

bool connect_db() {
    const char *db_username = getenv("db_username");
    if (db_username == NULL) db_username = DEFAULT_DB_USERNAME;

    mysql_init(&conn);
    if (!mysql_real_connect(&conn, NULL, db_username, getenv("db_pass"), DB, 0, NULL, 0)) {
        ERRORF("Failed to connect to database, error: %s\n", mysql_error(&conn));
        return false;
    }

    DEBUGF("Connected to DB as %s\n", db_username);
    not_connected = 0;

    return true;
}

void disconnect_db() {
    if (not_connected) return;

    mysql_close(&conn);
    not_connected = true;
}

MYSQL *get_connection() { return not_connected ? NULL : &conn; }
