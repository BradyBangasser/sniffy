#pragma once

#include <inttypes.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#include "errors.h"
#include "person.h"

typedef struct Charge Charge;

enum E_ArrestImpFlags {
    EAIF_PERSON_SET = 0x1,
    EAIF_BOND_SET = 0x2,
    EAIF_FAC_SET = 0x4,
    EAIF_RELEASE_DATE_SET = 0x08,
    EAIF_ARREST_DATE_SET = 0x10,
    EAIF_ID_SET = 0x20,
};

typedef struct {
    uint64_t id;
    uint32_t fid;
    union {
        uint8_t pid[32];
        Person *p;
    } _person;

    uint32_t bond;
    uint32_t initial_bond;

    Charge *charges;
    size_t n_charges;

    uint32_t _iflags;
} Arrest;

/**
 * Initialize the Arrest structure
 * This is required
 */
e_err arrest_init(Arrest *arr);

/**
 * Checks if the arrest is linked to a full person object
 */ 
static inline uint8_t arrest_has_full_person(Arrest *arr) { return arr->_person.p == NULL || arr->_iflags ^ EAIF_PERSON_SET; }

/**
 * Destroy the arrest structure
 */
e_err arrest_destroy(Arrest *arr);
