#pragma once

#include <inttypes.h>
#include <sys/types.h>

#include <mysql/mysql.h>

#include "errors.h"
#include "source.h"

typedef enum : char {
    SR_UNKNOWN,
    SR_BLACK,
    SR_WHITE,
    SR_ASIAN,
    SR_PACIFIC,
    SR_NATIVE,
} sniff_race;

typedef enum : char {
    S_UNKNOWN,
    S_FEMALE,
    S_MALE,
} sniff_sex;


enum sniff_e_piflags {
    EPIF_ID_SET  = 0x01,
    EPIF_FETCHED = 0x02,
    EPIF_SRC     = 0x04
};

/**
 * A person
 * @note After you set the name you are not able set the name again, you must use the person_update_name function
 * @note After you set the birth year you are not able set the birth year again, you must use the person_update_birth_year function
 */
typedef struct {
    uint8_t id[32];
    union {
        sniff_src *src;
        uint8_t *id;
    } src;

    char *first_name;
    char *middle_name;
    char *last_name;
    char *suffix;

    sniff_sex sex;
    sniff_race race;

    // Birth Year - 1900
    uint8_t birth_year; // Unset is 0
    uint8_t height; // In inches
    uint16_t weight; // In Pounds

    char *address;
    uint32_t phone_number; // Doesn't support international numbers

    char *notes;
    ssize_t notes_len;

    uint32_t _iflag;
} sniff_person;

sniff_e_err sniff_person_init(sniff_person *p);

sniff_e_err sniff_person_set_name(sniff_person *p, const char *first_name, const char *middle_name, const char *last_name, const char *suffix);
sniff_e_err sniff_person_set_birth_year(sniff_person *p, uint8_t birth_year);

sniff_e_err sniff_person_fetch_by_id(MYSQL *conn, uint8_t id[32], sniff_person *p);
sniff_e_err sniff_person_fetch_by_detail(MYSQL *conn, sniff_person *p);

sniff_e_err sniff_person_upsert(MYSQL *conn, sniff_person *p);

sniff_e_err sniff_person_destroy(sniff_person *p);
