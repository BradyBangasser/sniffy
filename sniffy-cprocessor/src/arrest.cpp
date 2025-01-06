#include "arrest.hpp"
#include "logging.h"

#include <algorithm>
#include <mysql/mysql.h>

uint64_t Arrest::id_c = 0;

bool Arrest::verify() {
    if (person == NULL || !person->verify()) return false;
    if (charges.size() < 1) return false;
    if (std::none_of(charges.begin(), charges.end(), [](Charge c) { return c.verify(); })) return false;

    return true;
}

bool Arrest::upsert(MYSQL *connection) {
    this->verify();
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wwritable-strings"
    static constexpr char *upsert_stmt = "INSERT INTO arrests (ID, PID, BookingAgencyID, Bond, InitialBond, ArrestDate) VALUES(?, ?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE `Bond` = VALUES(`Bond`), `UpdatedAt` = VALUES(`UpdatedAt`)";
    #pragma clang diagnostic pop

    uint8_t count = 0;

    if (!person->upsert(connection)) {
        ERRORF("Upsert on person (ID: %s) failed\n", person->id_to_str().c_str());
        return false;
    }

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

    uint64_t id_len = sizeof(this->person->id) / sizeof(uint8_t);
    bind[1].buffer_type = MYSQL_TYPE_BLOB;
    bind[1].buffer = this->person->id;
    bind[1].length = &id_len;
    bind[1].is_null = NULL;

    uint64_t ba_len = this->booking_agency.length();
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = const_cast<char *>(this->booking_agency.c_str());
    bind[2].length = &ba_len;
    bind[2].buffer_length = ba_len;
    bind[2].is_null = NULL;

    bind[3].buffer_type = MYSQL_TYPE_LONG;
    bind[3].buffer = &this->bond;
    bind[3].is_null = NULL;

    bind[4].buffer_type = MYSQL_TYPE_LONG;
    bind[4].buffer = &this->bond;
    bind[4].is_null = NULL;

    MYSQL_TIME ts;
    memset(&ts, 0, sizeof(ts));
    ts.year = 2024;
    ts.month = 2;
    ts.day = 2;
    ts.hour = 5;
    bind[5].buffer_type = MYSQL_TYPE_DATETIME;
    bind[5].is_null = NULL;
    bind[5].buffer = &ts;

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
    if (rows) SUCCESSF("%s Arrest %lx into database\n", rows - 1 ? "Inserted" : "Updated", id);

    std::for_each(this->charges.begin(), this->charges.end(), [](Charge c) {});

    return true;
}

Arrest::Arrest() : id{ Arrest::id_c++ }, docket_number{ 0 } {
    DEBUGF("Creating new arrest, id: %016lu\n", this->id);
    person = new Person();
}

Arrest::~Arrest() {
    delete person;
}
