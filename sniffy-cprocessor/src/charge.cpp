#include "charge.hpp"
#include "logging.h"

#include <time.h>
#include <mysql/mysql.h>
#include <numeric>

uint32_t Charge::id_c = 0;

Charge::Charge() {
    generate_id();
    DEBUGF("Created charge %lx\n", id);
}

bool Charge::verify() {
    if (!id) return false;
    if (statute.length() < 1) return false;
    if (statute_id.length() < 1) return false;
    if (timestamp.length() < 1) return false;
    return true;
}

bool Charge::generate_id() {
    struct tm t;
    uint64_t buf;

    id = 0;

    if (!timelocal(&t)) {
        ERROR("Failed to get datetime\n");
        return false;
    }

    id = (t.tm_mon - 1);
    id <<= 60;
    buf = t.tm_mday--;
    id |= (buf << 55);
    buf = t.tm_year - 100;
    id |= (buf << 48);
    buf = t.tm_min;
    id |= (buf << 42);
    buf = t.tm_sec;
    id |= (buf << 37);
    buf = getpid() % (1 << 5);
    id |= (buf << 32);
    id |= id_c++;

    return true;
}

bool Charge::upsert(MYSQL *connection) {
    this->verify();
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wwritable-strings"
    static constexpr char *upsert_stmt = "INSERT INTO charges (ID, AID, SID, DocketNumber, Bond, Notes) VALUES(?, ?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE `Bond` = VALUES(`Bond`)";
    #pragma clang diagnostic pop

    uint8_t count = 0;

    MYSQL_STMT *stmt;
    MYSQL_BIND bind[6];

    memset(bind, 0, sizeof(bind));

    stmt = mysql_stmt_init(connection);
    if (!stmt) {
        ERROR("Failed to initialize SQL statment, out of memory\n");
        return false;
    }

    if (mysql_stmt_prepare(stmt, upsert_stmt, strlen(upsert_stmt))) {
        ERRORF("Stmt perpare failure: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }

    count = mysql_stmt_param_count(stmt);

    if (count != sizeof(bind) / sizeof(MYSQL_BIND)) {
        ERRORF("Failed to upsert, expect %d params, got %ld\n", count, sizeof(bind) / sizeof(MYSQL_BIND));
        mysql_stmt_close(stmt);
        return false;
    }

    bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[0].buffer = (char *) &this->id;
    bind[0].is_null = NULL;
    bind[0].is_unsigned = true;
    bind[0].length = 0;

    bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[1].buffer = (char *) &this->arrest_id;
    bind[1].is_null = NULL;
    bind[1].is_unsigned = true;
    bind[1].length = 0;

    uint64_t ba_len = this->statute_id.length();
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = const_cast<char *>(this->statute_id.c_str());
    bind[2].length = &ba_len;
    bind[2].buffer_length = ba_len;
    bind[2].is_null = NULL;

    uint64_t di_len = this->docket_id.length();
    bool di_n = di_len == 0;
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = const_cast<char *>(this->docket_id.c_str());
    bind[3].length = &di_len;
    bind[3].buffer_length = di_len;
    bind[3].is_null = &di_n;

    bind[4].buffer_type = MYSQL_TYPE_LONG;
    bind[4].buffer = &this->bond;
    bind[4].is_null = NULL;

    std::string note_buf = std::reduce(this->notes.begin(), this->notes.end(), "", std::plus<std::string &>());
    bind[5].buffer_type = MYSQL_TYPE_STRING;
    bind[5].buffer = 

    if (mysql_stmt_bind_param(stmt, bind)) {
        ERRORF("Failed to bind stmt params, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }

    if (mysql_stmt_execute(stmt)) {
        ERRORF("Failed to execute stmt, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }

    uint64_t rows = mysql_stmt_affected_rows(stmt);

    mysql_stmt_close(stmt);
    if (rows) SUCCESSF("%s Charge %lx into database\n", rows - 1 ?  "Updated" : "Inserted", id);

    return true;
}
