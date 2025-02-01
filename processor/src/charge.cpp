#include "charge.hpp"
#include "logging.h"
#include "global.h"
#include "processor.hpp"

#include <time.h>
#include <mysql/mysql.h>
#include <numeric>

uint32_t Charge::id_c = 0;

Charge::Charge() : bond(0) {
    memset(&charged_at, 1, sizeof(struct tm));
}

Charge::Charge(uint64_t id, uint64_t aid, uint32_t bond, std::string sid, std::string docket, std::vector<std::string> notes) : bond(bond), id(id), arrest_id(aid), statute_id(sid), docket_id(docket), notes(notes) {
    memset(&this->charged_at, 1, sizeof(struct tm));
}

bool Charge::verify() {
    if (!id) return false;
    if (statute.length() < 1) return false;
    if (statute_id.length() < 1) return false;
    if (charged_at.tm_year == 1) return false;
    return true;
}

bool Charge::generate_id(struct FacilityData *fac_dat) {
    if (charged_at.tm_year == 1) return false;
    struct tm *tm;
    if (fac_dat && fac_dat->flags & RosterOptions::MO_INACCURATE_TIME) {
        const time_t t = time(NULL);
        tm = gmtime(&t);
    } else {
        tm = &charged_at;
    }
    uint64_t buf;

    id = 0;

    if (!tm) {
        ERROR("Failed to get datetime\n");
        return false;
    }

    id = tm->tm_mon;
    id <<= 60;
    buf = tm->tm_mday - 1;
    id |= (buf << 55);
    buf = tm->tm_year - 100;
    id |= (buf << 48);
    buf = tm->tm_min;
    id |= (buf << 42);
    buf = tm->tm_sec;
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
    // static constexpr char *upsert_stmt = "INSERT INTO charges (ID, AID, SID, DocketNumber, Bond, ChargedAt, Notes) VALUES(?, ?, ?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE `Bond` = VALUES(`Bond`)";
    static constexpr char *upsert_stmt = "CALL upsert_charge(?, ?, ?, ?, ?, ?, ?, ?)";
    #pragma clang diagnostic pop

    uint8_t count = 0;

    MYSQL_STMT *stmt;
    MYSQL_BIND bind[8];

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

    MYSQL_TIME ts;
    memset(&ts, 0, sizeof(ts));

    ts.year = this->charged_at.tm_year + 1900;
    ts.month = this->charged_at.tm_mon + 1;
    ts.day = this->charged_at.tm_mday;
    ts.hour = this->charged_at.tm_hour;
    ts.minute = this->charged_at.tm_min;
    ts.second = this->charged_at.tm_sec;
    bind[5].buffer_type = MYSQL_TYPE_DATETIME;
    bind[5].is_null = NULL;
    bind[5].buffer = &ts;

    std::string note_buf = std::reduce(this->notes.begin(), this->notes.end(), std::string(), [](std::string &a, std::string &b) { return a + "\n" + b; });
    uint64_t note_len = note_buf.length();
    bind[6].buffer_type = MYSQL_TYPE_STRING;
    bind[6].buffer = const_cast<char *>(note_buf.c_str()) + 1;
    bind[6].buffer_length = note_len;
    bind[6].length = &note_len;

    uint64_t stat_len = statute.length();
    bool stat_in = stat_len < 1;
    bind[7].buffer_type = MYSQL_TYPE_STRING;
    bind[7].buffer = const_cast<char *>(statute.c_str());
    bind[7].buffer_length = stat_len;
    bind[7].is_null = &stat_in;
    bind[7].length = &stat_len;

    if (mysql_stmt_bind_param(stmt, bind)) {
        ERRORF("Failed to bind stmt params, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }

    if (mysql_stmt_execute(stmt)) {
        ERRORF("Failed to insert charge, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }

    uint64_t rows = mysql_stmt_affected_rows(stmt);

    mysql_stmt_close(stmt);
    if (rows) SUCCESSF("%s Charge %lx into database\n", rows - 1 ?  "Updated" : "Inserted", id);

    return true;
}

std::vector<Charge> Charge::fetch(uint64_t aid, MYSQL *connection) {
    std::vector<Charge> charges;
    if (connection == NULL) return charges;

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wwritable-strings"
    static constexpr char *fetch_stmt = "SELECT Bond, DocketNumber, SID, Notes, ID FROM charges WHERE AID=?";
    #pragma clang diagnostic pop

    MYSQL_STMT *stmt;
    MYSQL_BIND bind[5];
    uint8_t docket[32] = { 0 };
    uint8_t sid[32] = { 0 };
    bool docket_in = true;
    uint32_t bond;
    uint64_t id;
    uint64_t notes_len;
    bool note_in = true;
    char *notes_c = 0, *split = 0;
    int status;

    memset(bind, 0, sizeof(bind));

    stmt = mysql_stmt_init(connection);
    if (stmt == NULL) {
        ERROR("Failed to init stmt, out of memory\n");
        return charges;
    }

    if (mysql_stmt_prepare(stmt, fetch_stmt, strlen(fetch_stmt))) {
        ERRORF("Failed to prepare stmt, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return charges;
    }

    bind[0].buffer = &aid;
    bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[0].is_unsigned = true;

    if (mysql_stmt_bind_param(stmt, bind)) {
        ERRORF("Failed to bind params, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return charges;
    }

    if (mysql_stmt_execute(stmt)) {
        ERRORF("Failed to execute stmt, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return charges;
    }

    mysql_stmt_store_result(stmt);
    if (mysql_stmt_num_rows(stmt) < 1) {
        WARNF("No (%lu) Charges linked with the AID %lx found\n", mysql_stmt_num_rows(stmt), aid);
        mysql_stmt_close(stmt);
        return charges;
    }

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &bond;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = docket;
    bind[1].buffer_length = 32;
    bind[1].is_null = &docket_in;

    bind[2].buffer = sid;
    bind[2].buffer_length = 32;
    bind[2].buffer_type = MYSQL_TYPE_STRING;

    bind[3].buffer = NULL;
    bind[3].length = &notes_len;
    bind[3].buffer_length = 0;
    bind[3].is_null = &note_in;

    bind[4].buffer = &id;
    bind[4].buffer_type = MYSQL_TYPE_LONGLONG;

    if (mysql_stmt_bind_result(stmt, bind)) {
        ERRORF("Failed to bind result, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return charges;
    }


    while ((status = mysql_stmt_fetch(stmt)) != 1 && status != MYSQL_NO_DATA) {
        std::vector<std::string> notes;
        if (!note_in && notes_len > 0) {
            notes_c = new char[notes_len];

            if (notes_c == NULL) {
                ERROR("Failed to allocation memory\n");
                mysql_stmt_close(stmt);
                return charges;
            }

            bind[3].buffer = notes_c;
            bind[3].buffer_length = notes_len;
            bind[3].is_null = &note_in;

            if (!mysql_stmt_fetch_column(stmt, bind, 3, 0)) {
                split = strtok(notes_c, "\n");
                while ((split = strtok(NULL, "\n")) != NULL) {
                    notes.push_back(split);
                }
            }

            delete[] notes_c;
            notes_c = 0;
        }

        charges.push_back(Charge(id, aid, bond, std::string((char *) sid), std::string(docket_in ? "" : (char *) docket), notes));
    }

    if (status == 1) {
        WARNF("Charge::fetch: Error fetching charges for AID %lx\n", aid);
    }

    return charges;
}


