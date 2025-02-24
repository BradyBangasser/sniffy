#ifndef SNIFFY_TEST_DATABASE_H
#define SNIFFY_TEST_DATABASE_H

#include <mysql/mysql.h>

#ifndef TEST_DB_HOST
#define TEST_DB_HOST NULL
#endif

#ifndef TEST_DB_UNAME
#define TEST_DB_UNAME "sniffy"
#endif

#ifndef TEST_DB_PREFIX
#define TEST_DB_PREFIX "sniffy"
#endif

uint8_t test_db_init(MYSQL *conn);
uint8_t test_db_destroy(MYSQL *conn);

#endif
