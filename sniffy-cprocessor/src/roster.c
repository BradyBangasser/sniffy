#include "roster.h"

#include <stdlib.h>
#include <memory.h>

struct RosterEntry *roster_create_entry(const uint8_t pid[32], const uint64_t aid) {
    struct RosterEntry *ent = malloc(sizeof(struct RosterEntry));
    if (ent == NULL) return NULL;
    memcpy((uint8_t *) ent->pid, pid, 32);
    *(uint64_t *) &ent->aid = aid;

    return ent;
}

void roster_free_entry(struct RosterEntry *ent) { free(ent); }

uint64_t _roster_hash_entry(const struct Roster *roster, const struct RosterEntry *ent) {
    uint64_t *pid_int = (uint64_t *) (ent->pid + 24);
    return *pid_int % roster->size;
}

uint8_t roster_insert(struct Roster *roster, const struct RosterEntry *ent) {
    if (roster_is_full(roster)) {
        uint32_t size = roster->size * 2;
        struct RosterEntry *npool = malloc(size * sizeof(struct RosterEntry));

        if (npool == NULL) {
            return 6;
        }

        
    }
}
