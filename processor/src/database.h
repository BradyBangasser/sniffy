#pragma once
#include <mysql/mysql.h>

#ifdef __cplusplus
namespace database {
    extern "C" {
#endif

bool connect_db();
void disconnect_db();
MYSQL *get_connection();

#ifdef __cplusplus
    }
}
#endif
