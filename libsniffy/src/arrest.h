#pragma once

#include <inttypes.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#include "errors.h"
#include "person.h"

typedef struct sniff_charge sniff_charge;

enum sniff_e_aiflags {
    EAIF_PERSON_SET         = 0x01,
    EAIF_BOND_SET           = 0x02,
    EAIF_FAC_SET            = 0x04,
    EAIF_RELEASE_DATE_SET   = 0x08,
    EAIF_ARREST_DATE_SET    = 0x10,
    EAIF_ID_SET             = 0x20,
};

typedef struct {
    uint64_t id;
    uint32_t fid;
    union {
        uint8_t pid[32];
        sniff_person *p;
    } _person;

    uint32_t bond;
    uint32_t initial_bond;

    sniff_charge *charges;
    size_t n_charges;

    uint32_t _iflags;
} sniff_arrest;

/**
 * Initialize the Arrest structure
 * This is required
 */
sniff_e_err sniff_arrest_init(sniff_arrest *arr);

/**
 * Checks if the arrest is linked to a full person object
 * returns 0 if the arrest contains a pointer to a full person
 */ 
static inline uint8_t sniff_arrest_has_full_person(sniff_arrest *arr) { return arr->_person.p == NULL || arr->_iflags ^ EAIF_PERSON_SET; }
static inline uint8_t sniff_arrest_has_pid(sniff_arrest *arr) { return arr->_person.p == NULL || arr->_iflags & EAIF_PERSON_SET; }
static inline sniff_person *sniff_arrest_get_person(sniff_arrest *arr) { return sniff_arrest_has_full_person(arr) ?  NULL : arr->_person.p; }

sniff_e_err sniff_arrest_set_charges(sniff_arrest *arr, sniff_charge *c, size_t n_charges);

/**
 * Destroy the arrest structure
 */
sniff_e_err sniff_arrest_destroy(sniff_arrest *arr);
