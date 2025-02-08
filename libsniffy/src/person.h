#pragma once

#include <inttypes.h>
#include <sys/types.h>

#include "errors.h"

typedef enum : char {
    SR_UNKNOWN = 0,
    SR_BLACK,
    SR_WHITE,
    SR_ASIAN,
    SR_PACIFIC,
    SR_NATIVE,
} Race;

typedef enum : char {
    S_UNKNOWN = 0,
    S_FEMALE,
    S_MALE,
} Sex;


enum E_PersonImpFlags {
    EPIF_NAME_SET = 0x1,
};

/**
 * A person
 * @note After you set the name you are not able set the name again, you must use the person_update_name function
 * @note After you set the birth year you are not able set the birth year again, you must use the person_update_birth_year function
 */
typedef struct {
    uint8_t id[32];

    const char *first_name;
    const char *middle_name;
    const char *last_name;
    const char *suffix;

    Sex sex;
    Race race;

    // Birth Year - 1900
    const uint8_t birth_year; // Unset is 0
    uint8_t height; // In inches
    uint16_t weight; // In Pounds

    char *address;
    uint32_t phone_number; // Doesn't support international numbers

    char *notes;
    ssize_t notes_len;

    uint32_t _iflag;
} Person;

e_err person_init(Person *p);

e_err person_destroy(Person *p);
