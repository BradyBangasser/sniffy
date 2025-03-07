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
        ns = strlen(name);

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
