#include "arrest.hpp"
#include "logging.h"

#include <algorithm>
#include <mysql/mysql.h>

uint16_t Arrest::id_c = 0;

bool Arrest::verify() {
    if (person == NULL || !person->verify()) return false;
    if (arrested_at.tm_year == 1) return false;
    if (charges.size() < 1) return false;
    if (std::none_of(charges.begin(), charges.end(), [](Charge c) { return c.verify(); })) return false;

    return true;
}

bool Arrest::generate_id() {
    struct tm *tm = &arrested_at;
    id = 0;
    uint64_t buf;

    if (arrested_at.tm_year == 1) {
       return false;
    }

    if (!tm) {
        ERROR("Failed to get datetime\n");
        return false;
    }

    id = tm->tm_year - 100;
    id <<= 57;
    buf = tm->tm_mon;
    id |= buf << 53;
    buf = tm->tm_mday - 1;
    id |= buf << 48;
    buf = tm->tm_hour;
    id |= buf << 43;
    buf = tm->tm_min;
    id |= buf << 36;
    buf = tm->tm_sec;
    id |= buf << 29;
    buf = getpid() % 0x2000;
    id |= buf << 16;
    id |= Arrest::id_c++;

    return true;
}

bool Arrest::upsert(MYSQL *connection) {
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wwritable-strings"
    static constexpr char *upsert_stmt = "INSERT INTO arrests (ID, PID, BookingAgencyID, Bond, InitialBond, ArrestDate, ReleaseDate, Notes, FID) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE `Bond` = VALUES(`Bond`), `UpdatedAt` = VALUES(`UpdatedAt`)";
    #pragma clang diagnostic pop

    uint8_t count = 0;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[9];
    MYSQL_TIME arrest_ts, release_ts;
    bool rd_in = true;

    this->verify();

    if (person && !person->upsert(connection)) {
        ERRORF("Upsert on person (ID: %s) failed\n", person->id_to_str().c_str());
        return false;
    }

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
    bind[1].buffer = person ? this->person->id : this->pid;
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

    memset(&arrest_ts, 0, sizeof(arrest_ts));

    arrest_ts.year = this->arrested_at.tm_year + 1900;
    arrest_ts.month = this->arrested_at.tm_mon + 1;
    arrest_ts.day = this->arrested_at.tm_mday;
    arrest_ts.hour = this->arrested_at.tm_hour;
    arrest_ts.minute = this->arrested_at.tm_min;
    arrest_ts.second = this->arrested_at.tm_sec;
    bind[5].buffer_type = MYSQL_TYPE_DATETIME;
    bind[5].is_null = NULL;
    bind[5].buffer = &arrest_ts;

    if (this->release_date.tm_year > 0) {
        rd_in = false;
        memset(&release_ts, 0, sizeof(arrest_ts));

        release_ts.year = this->release_date.tm_year + 1900;
        release_ts.month = this->release_date.tm_mon + 1;
        release_ts.day = this->release_date.tm_mday;
        release_ts.hour = this->release_date.tm_hour;
        release_ts.minute = this->release_date.tm_min;
        release_ts.second = this->release_date.tm_sec;
    }

    bind[6].buffer_type = MYSQL_TYPE_DATETIME;
    bind[6].is_null = &rd_in;
    bind[6].buffer = &release_ts;

    std::string note_buf = std::reduce(this->notes.begin(), this->notes.end(), std::string(), [](std::string &a, std::string &b) { return a + "\n" + b; });
    uint64_t note_len = note_buf.length();
    bind[7].buffer_type = MYSQL_TYPE_STRING;
    bind[7].buffer = const_cast<char *>(note_buf.c_str()) + 1;
    bind[7].buffer_length = note_len;
    bind[7].length = &note_len;

    bind[8].buffer_type = MYSQL_TYPE_LONG;
    bind[8].buffer = &this->fac_id;
    bind[8].is_unsigned = true;
    DEBUG("HERE\n");

    if (mysql_stmt_bind_param(stmt, bind)) {
        ERRORF("Failed to bind stmt params, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }

    if (mysql_stmt_execute(stmt)) {
        ERRORF("Failed to execute stmt, error: %s\nAID: %lu\n", mysql_stmt_error(stmt), this->id);
        mysql_stmt_close(stmt);
        return false;
    }

    uint64_t rows = mysql_stmt_affected_rows(stmt);

    mysql_stmt_close(stmt);
    if (rows) SUCCESSF("%s Arrest %lx into database\n", rows - 1 ?  "Updated" : "Inserted", id);

    std::for_each(this->charges.begin(), this->charges.end(), [connection](Charge c) { c.upsert(connection); });

    return true;
}

Arrest *Arrest::fetch(uint64_t id, MYSQL *connection) {
    if (connection == NULL) return NULL;

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wwritable-strings"
    static constexpr char *fetch_stmt = "SELECT PID, Bond, Notes FROM arrests WHERE ID=?";
    #pragma clang diagnostic pop

    MYSQL_STMT *stmt;
    MYSQL_BIND bind[3];
    uint8_t pid[32] = { 0 };
    uint32_t bond;
    uint64_t notes_len;
    bool note_in = true;
    char *notes_c = 0, *split = 0;
    std::vector<std::string> notes;

    memset(bind, 0, sizeof(bind));

    stmt = mysql_stmt_init(connection);
    if (stmt == NULL) {
        ERROR("Failed to init stmt, out of memory\n");
        return NULL;
    }

    if (mysql_stmt_prepare(stmt, fetch_stmt, strlen(fetch_stmt))) {
        ERRORF("Failed to prepare stmt, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return NULL;
    }

    bind[0].buffer = &id;
    bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[0].is_unsigned = true;

    if (mysql_stmt_bind_param(stmt, bind)) {
        ERRORF("Failed to bind params, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return NULL;
    }

    if (mysql_stmt_execute(stmt)) {
        ERRORF("Failed to execute stmt, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return NULL;
    }

    mysql_stmt_store_result(stmt);
    if (mysql_stmt_num_rows(stmt) < 1) {
        WARNF("No Arrest with the ID %lx found\n", id);
        mysql_stmt_close(stmt);
        return NULL;
    }

    bind[0].buffer_type = MYSQL_TYPE_BLOB;
    bind[0].buffer = pid;
    bind[0].buffer_length = 32;

    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &bond;

    bind[2].buffer = NULL;
    bind[2].length = &notes_len;
    bind[2].buffer_length = 0;
    bind[2].is_null = &note_in;

    if (mysql_stmt_bind_result(stmt, bind)) {
        ERRORF("Failed to bind result, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return NULL;
    }

    mysql_stmt_fetch(stmt);

    if (!note_in && notes_len > 0) {
        notes_c = new char[notes_len];

        bind[2].buffer = notes_c;
        bind[2].buffer_length = notes_len;

        if (!mysql_stmt_fetch_column(stmt, bind, 2, 0)) {
            split = strtok(notes_c, "\n");
            while ((split = strtok(NULL, "\n")) != NULL) {
                notes.push_back(split);
            }
        }

        delete[] notes_c;
        notes_c = 0;
    }

    Arrest *arr = new Arrest(id, pid, bond, notes);
    arr->charges = Charge::fetch(id, connection);
    return arr;
}

Arrest::Arrest(uint64_t id, uint8_t pid[32], uint32_t bond, std::vector<std::string> notes) : id(id), bond(bond), initial_bond(bond), person(NULL), notes(notes) {
    this->pid = new uint8_t[32];
    memcpy(this->pid, pid, 32);
    memset(&arrested_at, 1, sizeof(struct tm));
    memset(&release_date, 0, sizeof(struct tm));
}

Arrest::Arrest() : pid(NULL) {
    this->generate_id();
    memset(&arrested_at, 1, sizeof(struct tm));
    memset(&release_date, 0, sizeof(struct tm));
    person = new Person();
}

Arrest::~Arrest() {
    if (pid != NULL) delete pid;
    if (person != NULL) delete person;
}
