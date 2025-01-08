#pragma once

#include <stdint.h>
#include <mysql/mysql.h>

#define MAX_LOAD_FACTOR 80
#define MIN_LOAD_FACTOR 20
#define INITIAL_SIZE 100
#define OPEN_ADDR_JUMP 1

#ifdef __cplusplus
extern "C" {
#endif

struct RosterEntry {
    const uint8_t pid[32];   
    const uint64_t aid;
};

struct Roster {
    struct RosterEntry **pool;
    uint32_t size;
    uint32_t length;
};

static inline bool roster_is_full(const struct Roster *roster) {
    return ((roster->length * 100) / (roster->size * 100)) >= MAX_LOAD_FACTOR;
}

static inline bool roster_is_empty(const struct Roster *roster) {
    return roster->size > INITIAL_SIZE && ((roster->length * 100) / (roster->size * 100)) <= MIN_LOAD_FACTOR;
}

struct RosterEntry *roster_create_entry(const uint8_t pid[32], const uint64_t aid);
void roster_free_entry(struct RosterEntry *ent);

uint64_t _roster_hash_entry(const struct Roster *roster, const uint8_t pid[32]);
// NOTE: ent is not copied, so do not remove it from memory
uint8_t roster_insert(struct Roster *roster, struct RosterEntry *ent);
const struct RosterEntry *roster_get(const struct Roster *roster, const uint8_t pid[32]);
// NOTE: 
struct RosterEntry *roster_remove(struct Roster *roster, const uint8_t pid[32]);
struct Roster *roster_create();
void roster_free(struct Roster *roster);

struct Roster *fetch_roster(MYSQL *connection);

#ifdef __cplusplus
}
#endif
