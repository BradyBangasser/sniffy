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

#define STR(S) #S,
#define EN(S) S,

static const char *err_codes[] = { ALL_ERRORS(STR) };

typedef enum {
    ALL_ERRORS(EN)
} e_err;
