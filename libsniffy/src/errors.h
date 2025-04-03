#pragma once

#define ALL_ERRORS(F) \
    F(ERR_OK) /* Everything was OK */ \
    F(ERR_NOT_FOUND) \
    F(ERR_NOT_IMPLEMENTED) \
    F(ERR_NOT_ENOUGH_DATA) \
    F(ERR_MYSQL_STMT_INIT) /* Failure allocating memory for a MYSQL STMT struct */ \
    F(ERR_MYSQL_STMT_PREPARE) \
    F(ERR_MYSQL_STMT_BIND_PARAM) \
    F(ERR_MYSQL_STMT_EXE_FAILURE) \
    F(ERR_MYSQL_STMT_RESULT_BIND_FAILURE) \
    F(ERR_ALLOC_FAILURE) \
    F(ERR_SSL_DIGEST_UPDATE_FAILURE) \
    F(ERR_SSL_DIGEST_FINAL_FAILURE) \
    F(ERR_SSL_DIGEST_INIT_FAILURE)

#define STR(S) #S,
#define EN(S) S,

static const char *sniff_err_codes[] = { ALL_ERRORS(STR) };

typedef enum {
    ALL_ERRORS(EN)
} sniff_e_err;

static inline const char *sniff_err_to_str(sniff_e_err ecode) { return sniff_err_codes[ecode]; }
