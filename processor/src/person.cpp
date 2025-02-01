#include "person.hpp"
#include "stringification.hpp"
#include "logging.h"

#include <openssl/evp.h>
#include <format>
#include <numeric>

Person::Person() : suffix(NULL), id{ 0 } {
}

bool Person::generate_id() {
    if (!verify(false)) return false;

    EVP_MD_CTX *ctx;
    const EVP_MD *md;
    uint32_t len;
    id_set = false;
    memset(id, 0, sizeof(id));

    md = EVP_get_digestbyname("SHA256");

    if (md == NULL) {
        ERROR("Failed to find digest by name\n");
        return false;
    }

    ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        ERROR("Failed to create MD context\n");
        return false;
    }

    if (!EVP_DigestInit_ex2(ctx, md, NULL)) {
        ERROR("Failed to initialize digest\n");
        EVP_MD_CTX_free(ctx);
        return false;
    }

    std::string lfn = stringification::lower_str(this->first_name);
    if (!EVP_DigestUpdate(ctx, lfn.c_str(), lfn.length())) {
        ERROR("Failed to hash firstname\n");
        EVP_MD_CTX_free(ctx);
        return false;
    }

    std::string lln = stringification::lower_str(this->last_name);
    if (!EVP_DigestUpdate(ctx, lln.c_str(), lln.length())) {
        ERROR("Failed to hash lastname\n");
        EVP_MD_CTX_free(ctx);
        return false;
    }

    if (this->middle_name.length()) {
        std::string lmn = stringification::lower_str(this->middle_name);
        if (!EVP_DigestUpdate(ctx, lmn.c_str(), lmn.length())) {
            ERROR("Failed to hash middlename\n");
            EVP_MD_CTX_free(ctx);
            return false;
        }
    }

    if (this->suffix && this->suffix->length() > 0) {
        std::string ls = stringification::lower_str(*this->suffix);
        if (!EVP_DigestUpdate(ctx, ls.c_str(), ls.length())) {
            ERROR("Failed to hash suffix\n");
            EVP_MD_CTX_free(ctx);
            return false;
        }
    }

    if (!EVP_DigestUpdate(ctx, &this->birth_year, sizeof(uint16_t))) {
        ERROR("Failed to hash birthyear\n");
        EVP_MD_CTX_free(ctx);
        return false;
    }

    if (!EVP_DigestFinal_ex(ctx, id, &len)) {
        ERROR("Digest final failed\n");
        EVP_MD_CTX_free(ctx);
        return false;
    }

    EVP_MD_CTX_free(ctx);

    id_set = true;
    return true;
}

bool Person::verify(bool genId) {
    if (this->first_name.length() < 1) return false;
    if (this->last_name.length() < 1) return false;

    if (genId && !id_set && !generate_id()) {
        WARN("Failed to generate ID\n");
        return false;
    }

    return true;
}

std::string Person::id_to_str(uint8_t id[32]) {
    std::string s;
    for (uint8_t i = 0; i < 32; i++) {
        s += std::format("{:02x}", id[i]);
    }
    return s;
}

uint32_t Person::parse_height(const char *str) {
    const char *curs;
    char nums[4] = { 0 };
    uint8_t n_i = 0;

    curs = str;

    while (curs != NULL) {
        if (*curs >= 0x30 && *curs < 0x3A) {
            nums[n_i++] = *curs;
            if (n_i == sizeof(nums) - 1) break;
        }
        curs++;
    }

    if (!n_i) return 0;

    if (n_i == 1) {
        // assume feet
        return (nums[0] - 0x30) * 12;
    } else if (n_i == 2) { 
        // assume inches
        return atoi(nums);
    } else if (n_i == 3) {
        if ((nums[0] - 0x30) >= 4) {
            // assume feet/inches
            return (nums[0] - 0x30) * 12 + atoi(nums + 1);
        } else if ((nums[0] - 0x30) == 0) {
            // inches again
        return atoi(nums);
        } else {
            // whatever the metric system is
            return atoi(nums) / 2.54f;
        }
    } else {
        return 0;
    }
}

bool Person::upsert(MYSQL *connection) {
    this->verify();
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wwritable-strings"
    static constexpr char *upsert_stmt = "INSERT INTO people (ID, FirstName, LastName, MiddleName, Suffix, Height, Weight, Race, Sex, BirthYear, Address, PhoneNumber, Notes) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE `Height` = VALUES(`Height`), `Weight` = VALUES(`Weight`)";
    #pragma clang diagnostic pop

    uint8_t count = 0;

    MYSQL_STMT *stmt;
    MYSQL_BIND bind[13];

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

    uint64_t id_len = 32;
    bind[0].buffer_type = MYSQL_TYPE_BLOB;
    bind[0].buffer = (char *) this->id;
    bind[0].is_null = NULL;
    bind[0].length = &id_len;
    bind[0].buffer_length = id_len;

    uint64_t fn_len = this->first_name.length();
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = const_cast<char *>(this->first_name.c_str());
    bind[1].length = &fn_len;
    bind[1].buffer_length = fn_len;
    bind[1].is_null = NULL;

    uint64_t ln_len = this->last_name.length();
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = const_cast<char *>(this->last_name.c_str());
    bind[2].length = &ln_len;
    bind[2].buffer_length = ln_len;
    bind[2].is_null = NULL;

    uint64_t mn_len = this->middle_name.length();
    bool mn_n = this->middle_name.length() == 0;
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = mn_n ?  NULL : const_cast<char *>(this->middle_name.c_str());
    bind[3].length = &mn_len;
    bind[3].buffer_length = mn_len;
    bind[3].is_null = &mn_n;

    bool sf_n = !this->suffix || this->suffix->length() == 0;
    uint64_t sf_len = sf_n ? 0 : this->suffix->length();
    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = sf_n ? NULL : const_cast<char *>(this->suffix->c_str());
    bind[4].length = &sf_len;
    bind[4].buffer_length = sf_len;
    bind[4].is_null = &sf_n;

    bind[5].buffer_type = MYSQL_TYPE_TINY;
    bind[5].buffer = &this->height;
    bind[5].is_unsigned = true;
    bind[5].is_null = NULL;

    bind[6].buffer_type = MYSQL_TYPE_SHORT;
    bind[6].buffer = &this->weight;
    bind[6].is_unsigned = true;
    bind[6].is_null = NULL;

    short int r = this->race;
    bind[7].buffer_type = MYSQL_TYPE_SHORT;
    bind[7].buffer = &r;
    bind[7].is_null = NULL;

    bind[8].buffer_type = MYSQL_TYPE_TINY;
    bind[8].buffer = &this->sex;
    bind[8].is_null = NULL;

    bind[9].buffer_type = MYSQL_TYPE_SHORT;
    bind[9].buffer = &this->birth_year;

    bool n = true;
    bind[10].buffer_type = MYSQL_TYPE_STRING;
    bind[10].is_null = &n;
    bind[11].buffer_type = MYSQL_TYPE_STRING;
    bind[11].is_null = &n;

    std::string note_buf = std::reduce(this->notes.begin(), this->notes.end(), std::string(), [](std::string &a, std::string &b) { return a + "\n" + b; });
    uint64_t note_len = note_buf.length();
    bind[12].buffer_type = MYSQL_TYPE_STRING;
    bind[12].buffer = const_cast<char *>(note_buf.c_str()) + 1;
    bind[12].buffer_length = note_len;
    bind[12].length = &note_len;

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
    if (rows) SUCCESSF("%s %s %s (ID: %s) into database\n", rows - 1 ? "Updated" : "Inserted", this->first_name.c_str(), this->last_name.c_str(), this->id_to_str().c_str());
    return true;
}
