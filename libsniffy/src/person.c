#include <memory.h>
#include <assert.h>

#include <openssl/evp.h>
#include <mysql/mysql.h>

#include "person.h"
#include "errors.h"

e_err person_init(Person *p) {
    memset(p, 0, sizeof(*p));
    return ERR_OK;
}

static e_err _person_generate_id(Person *p) {
    if (!p->first_name || !p->middle_name || !p->last_name || p->birth_year == 0) {
        return ERR_NOT_ENOUGH_DATA;
    }

    EVP_MD_CTX *ctx;
    const EVP_MD *md;
    ssize_t len;

    memset((void *) p->id, 0, sizeof(p->id));

    md = EVP_get_digestbyname("SHA256");

    if (md == NULL) {
        return ERR_SSL_DIGEST_INIT_FAILURE;
    }

    ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        return ERR_SSL_DIGEST_UPDATE_FAILURE;
    }

    if (!EVP_DigestInit_ex2(ctx, md, NULL)) {
        EVP_MD_CTX_free(ctx);
        return ERR_SSL_DIGEST_UPDATE_FAILURE;
    }

    if (!EVP_DigestUpdate(ctx, p->first_name, strlen(p->first_name))) {
        EVP_MD_CTX_free(ctx);
        return ERR_SSL_DIGEST_UPDATE_FAILURE;
    }

    if (!EVP_DigestUpdate(ctx, p->last_name, strlen(p->last_name))) {
        EVP_MD_CTX_free(ctx);
        return ERR_SSL_DIGEST_UPDATE_FAILURE;
    }

    if (p->middle_name && (len = strlen(p->middle_name))) {
        if (!EVP_DigestUpdate(ctx, p->middle_name, len)) {
            EVP_MD_CTX_free(ctx);
            return ERR_SSL_DIGEST_UPDATE_FAILURE;
        }
    }

    if (p->suffix && (len = strlen(p->suffix))) {
        if (!EVP_DigestUpdate(ctx, p->suffix, len)) {
            EVP_MD_CTX_free(ctx);
            return ERR_SSL_DIGEST_UPDATE_FAILURE;
        }
    }

    if (!EVP_DigestUpdate(ctx, &p->birth_year, sizeof(p->birth_year))) {
        EVP_MD_CTX_free(ctx);
        return ERR_SSL_DIGEST_UPDATE_FAILURE;
    }

    if (!EVP_DigestUpdate(ctx, &p->sex, sizeof(p->sex))) {
        EVP_MD_CTX_free(ctx);
        return ERR_SSL_DIGEST_UPDATE_FAILURE;
    }

    if (!EVP_DigestFinal_ex(ctx, (void *) p->id, (uint32_t *) &len)) {
        EVP_MD_CTX_free(ctx);
        return ERR_SSL_DIGEST_FINAL_FAILURE;
    }

    EVP_MD_CTX_free(ctx);

    return ERR_OK;
}

static inline e_err _person_set_name(const char **oname, const char *name) {
    size_t ns;
    if (name) {
        ns = strlen(name) + 1;

        if (!(*oname = *oname ? realloc((void *) *oname, ns) : malloc(ns))) {
            return ERR_ALLOC_FAILURE;
        }

        strcpy((void *) *oname, name);
    }

    return ERR_OK;
}

e_err person_set_name(Person *p, const char *first_name, const char *middle_name, const char *last_name, const char *suffix) {
    if (p->_iflag & EPIF_FETCHED) {
        assert(0 && "Modifying names on fetched person is not implemented yet");
        return ERR_NOT_IMPLEMENTED;
    }

    if (
        _person_set_name(&p->first_name, first_name) ||
        _person_set_name(&p->middle_name, middle_name) || 
        _person_set_name(&p->last_name, last_name) ||
        _person_set_name(&p->suffix, suffix)
       ) {
        return ERR_ALLOC_FAILURE;
    }

    _person_generate_id(p);

    return ERR_OK;
}

e_err person_fetch_by_id(MYSQL *conn, uint8_t id[32], Person *p) {
    static const char *query = "SELECT FirstName, MiddleName, LastName, Suffix, Sex, Race, Height, Weight, Address, PhoneNumber, Note FROM people WHERE ID=?";

    uint64_t fn_l, mn_l, ln_l, sf_l, ad_l, no_l;

    MYSQL_BIND bind[11];
    MYSQL_STMT *stmt;

    memset(bind, 0, sizeof(bind));

    bind[0].buffer = id;
    bind[0].buffer_length = 32;

    if (!(stmt = mysql_stmt_init(conn))) {
        return ERR_MYSQL_STMT_INIT;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        return ERR_MYSQL_STMT_PREPARE;
    }

    if (mysql_stmt_bind_param(stmt, bind)) {
        return ERR_MYSQL_STMT_BIND_PARAM;
    }

    if (mysql_stmt_execute(stmt)) {
        return ERR_MYSQL_STMT_EXE_FAILURE;
    }

    bind[0].buffer = 0;
    bind[0].buffer_length = 0;
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].length = &fn_l;

    bind[1].buffer = 0;
    bind[1].buffer_length = 0;
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].length = &mn_l;

    bind[2].buffer = 0;
    bind[2].buffer_length = 0;
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].length = &ln_l;

    bind[3].buffer = 0;
    bind[3].buffer_length = 0;
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].length = &sf_l;

    bind[4].buffer = (void *) &p->sex;
    bind[4].buffer_type = MYSQL_TYPE_TINY;
    bind[5].buffer = &p->race;
    bind[5].buffer_type = MYSQL_TYPE_TINY;
    bind[6].buffer = &p->height;
    bind[6].buffer_type = MYSQL_TYPE_TINY;
    bind[6].is_unsigned = true;
    bind[7].buffer = &p->weight;
    bind[7].buffer_type = MYSQL_TYPE_SHORT;
    bind[7].is_unsigned = true;

    bind[8].buffer = 0;
    bind[8].buffer_type = MYSQL_TYPE_STRING;
    bind[8].buffer_length = 0;
    bind[8].length = &ad_l;

    bind[9].buffer = &p->phone_number;
    bind[9].buffer_type = MYSQL_TYPE_LONG;
    bind[9].is_unsigned = true;

    bind[10].buffer = 0;
    bind[10].buffer_type = MYSQL_TYPE_STRING;
    bind[10].buffer_length = 0;
    bind[10].length = &no_l;

    if (mysql_stmt_bind_result(stmt, bind)) {
        return ERR_MYSQL_STMT_RESULT_BIND_FAILURE;
    }

    if (mysql_stmt_fetch(stmt) == MYSQL_NO_DATA) {
        return ERR_NOT_FOUND;
    }

    p->first_name = calloc(fn_l + 1, sizeof(char));
    if (p->first_name == NULL) {
        person_destroy(p);
        return ERR_ALLOC_FAILURE;
    }

    bind[0].buffer = (void *) p->first_name;
    bind[0].buffer_length = fn_l;

    p->middle_name = calloc(mn_l + 1, sizeof(char));
    if (p->middle_name == NULL) {
        person_destroy(p);
        return ERR_ALLOC_FAILURE;
    }

    bind[1].buffer = (void *) p->middle_name;
    bind[1].buffer_length = mn_l;

    p->last_name = calloc(ln_l + 1, sizeof(char));
    if (p->last_name == NULL) {
        person_destroy(p);
        return ERR_ALLOC_FAILURE;
    }

    bind[2].buffer = (void *) p->last_name;
    bind[2].buffer_length = ln_l;

    if (sf_l) {
        p->suffix = calloc(sf_l + 1, sizeof(char));
        if (p->suffix == NULL) {
            person_destroy(p);
            return ERR_ALLOC_FAILURE;
        }

        bind[3].buffer = (void *) p->suffix;
        bind[3].buffer_length = sf_l;
    }

    if (ad_l) {
        p->address = calloc(ad_l + 1, sizeof(char));
        if (p->address == NULL) {
            person_destroy(p);
            return ERR_ALLOC_FAILURE;
        }

        bind[8].buffer = (void *) p->address;
        bind[8].buffer_length = ad_l;
    }

    if (no_l) {
        p->notes = calloc(no_l + 1, sizeof(char));
        if (p->notes == NULL) {
            person_destroy(p);
            return ERR_ALLOC_FAILURE;
        }

        bind[10].buffer = (void *) p->notes;
        bind[10].buffer_length = no_l;
    }

    if (
            mysql_stmt_fetch_column(stmt, bind, 0, 0) ||
            mysql_stmt_fetch_column(stmt, bind, 1, 0) ||
            mysql_stmt_fetch_column(stmt, bind, 2, 0) ||
            mysql_stmt_fetch_column(stmt, bind, 3, 0) ||
            mysql_stmt_fetch_column(stmt, bind, 8, 0) ||
            mysql_stmt_fetch_column(stmt, bind, 10, 0)
       ) {
        return ERR_MYSQL_STMT_EXE_FAILURE;
    }

    memcpy((void *) p->id, id, sizeof(p->id) / sizeof(p->id[0]));
    
    p->_iflag |= EPIF_FETCHED;

    return ERR_OK;
}

e_err person_fetch_by_detail(MYSQL *conn, Person *p) {
    assert(0 && "Not implemented yet");
    return ERR_NOT_IMPLEMENTED;
}

static inline uint8_t _person_not_partial(Person *p) {
    return p->first_name && p->last_name && p->middle_name && p->birth_year && p->birth_year;
}

e_err person_upsert(MYSQL *conn, Person *p) {
    e_err err = ERR_OK;
    const static char *insert = "INSERT INTO people ("
        "id,"
        "first_name,"
        "middle_name," 
        "last_name," 
        "suffix,"
        "sex," 
        "race," 
        "birth_year,"
        "height,"
        "weight,"
        "address,"
        "phone_number,"
        "notes"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    static MYSQL_BIND bind[13];

    MYSQL_STMT *stmt;

    memset(bind, 0, sizeof(bind));

    if ((stmt = mysql_stmt_init(conn)) == NULL) {
        return ERR_ALLOC_FAILURE;
    }

    if (mysql_stmt_prepare(stmt, insert, strlen(insert))) {
        err = ERR_MYSQL_STMT_PREPARE;
        goto close;
    }

    bind[0].buffer = (void *) p->id;
    bind[0].buffer_type = MYSQL_TYPE_BLOB;
    bind[0].buffer_length = 32;

    bind[1].buffer = (void *) p->first_name;
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].is_null = (void *) 0;

    bind[2].buffer = (void *) p->middle_name;
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].is_null = (void *) (uint64_t) ((intptr_t) p->middle_name <= 1);

    bind[3].buffer = (void *) p->last_name;
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].is_null = (void *) 0;

    bind[4].buffer = (void *) p->suffix;
    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].is_null = (void *) (uint64_t) ((intptr_t) p->suffix <= 1);

    bind[5].buffer = (void *) &p->sex;
    bind[5].buffer_type = MYSQL_TYPE_TINY;
    bind[5].is_null = (void *) &p->sex;

    bind[6].buffer = &p->race;
    bind[6].buffer_type = MYSQL_TYPE_TINY;
    bind[6].is_null = (void *) &p->race;

    bind[7].buffer = (void *) &p->birth_year;
    bind[7].buffer_type = MYSQL_TYPE_TINY;
    bind[7].is_unsigned = 1;
    bind[7].is_null = (void *) 0;

    bind[8].buffer = (void *) &p->weight;
    bind[8].buffer_type = MYSQL_TYPE_TINY;
    bind[8].is_unsigned = 1;
    // World record for lowest weight is 4.7 or something
    bind[8].is_null = (void *) (uint64_t) (p->weight < 3);

    bind[9].buffer = (void *) &p->height;
    bind[9].buffer_type = MYSQL_TYPE_SHORT;
    bind[9].is_unsigned = 1;
    // Shortest height was 1'9 or 21"
    bind[9].is_null = (void *) (uint64_t) (p->height < 20);

    bind[10].buffer = (void *) p->address;
    bind[10].buffer_type = MYSQL_TYPE_STRING;
    bind[10].is_null = (void *) p->address;

    bind[11].buffer = (void *) &p->phone_number;
    bind[11].buffer_type = MYSQL_TYPE_LONG;
    bind[11].is_null = (void *) &p->phone_number;

    bind[12].buffer = (void *) p->notes;
    bind[12].buffer_type = MYSQL_TYPE_STRING;
    bind[12].is_null = (void *) p->notes;

    if (mysql_stmt_bind_param(stmt, bind)) {
        err = ERR_MYSQL_STMT_BIND_PARAM;
        goto close;
    }
    
    if (mysql_stmt_execute(stmt)) {
        // This may fail due to a duplicate key, we need to handle that
        assert(0 && "TODO");
        err = ERR_MYSQL_STMT_EXE_FAILURE;
        goto close;
    }

close:
    mysql_stmt_close(stmt);

    return err;
}

e_err person_destroy(Person *p) {
    if (p->first_name) free((void *) p->first_name);
    if (p->middle_name) free((void *) p->middle_name);
    if (p->last_name) free((void *) p->last_name);
    if (p->suffix) free((void *) p->suffix);
    if (p->address) free(p->address);
    if (p->notes) free(p->notes);
    memset(p, 0, sizeof(*p));
    return ERR_OK;
}
