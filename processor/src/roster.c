#include "roster.h"
#include "logging.h"

#include <stdlib.h>
#include <memory.h>
#include <mysql/mysql.h>

struct Roster *roster_create() {
    struct Roster *roster = malloc(sizeof(struct Roster));
    if (roster == NULL) {
        return NULL;
    }

    roster->pool = calloc(sizeof(struct RosterEntry *), INITIAL_SIZE);
    if (roster->pool == NULL) {
        free(roster);
        return NULL;
    }

    roster->size = INITIAL_SIZE;
    roster->length = 0;

    return roster;
}

void roster_free(struct Roster *roster) {
    for (int i = 0; i < roster->size; i++) {
        if (roster->pool[i] > (struct RosterEntry *) 1) {
            free(roster->pool[i]);
        }
    }

    free(roster->pool);
    free(roster);
}

struct RosterEntry *roster_create_entry(const uint8_t pid[32], const uint64_t aid) {
    struct RosterEntry *ent = calloc(1, sizeof(struct RosterEntry));
    if (ent == NULL) return NULL;
    *(uint64_t *) &ent->aid = aid;
    memcpy((uint8_t *) ent->pid, pid, 32);

    return ent;
}

void roster_free_entry(struct RosterEntry *ent) { free(ent); }

uint64_t _roster_hash_entry(const struct Roster *roster, const uint8_t pid[32]) {
    uint64_t *pid_int = (uint64_t *) (pid + 24);
    return *pid_int % roster->size;
}

uint8_t roster_insert(struct Roster *roster, struct RosterEntry *ent) {
    uint32_t i = 0;
    if (roster_is_full(roster)) {
        uint32_t osize = roster->size;
        struct RosterEntry **opool = roster->pool;
        roster->size *= 2;
        roster->pool = malloc(sizeof(struct RosterEntry *) * roster->size);

        if (roster->pool == NULL) {
            return 6;
        }

        memset(roster->pool, 0, roster->size * sizeof(struct RosterEntry *));

        for (i = 0; i < osize; i++) {
            if ((uint64_t) opool[i] > 1) roster_insert(roster, opool[i]);
        }

        free(opool);
    }

    uint32_t hashval = _roster_hash_entry(roster, ent->pid);
    
    while ((uint64_t) roster->pool[hashval] > 1) {
        hashval = (hashval + OPEN_ADDR_JUMP) % roster->size;
    }

    roster->pool[hashval] = ent;
    roster->length++;

    return true;
}

const struct RosterEntry *roster_get(const struct Roster *roster, const uint8_t pid[32]) {
    uint64_t hashval = _roster_hash_entry(roster, pid);
    uint32_t i = hashval;

    do {
        if (roster->pool[i] == NULL) return NULL;
        if (roster->pool[i] != (void *) 1 && memcmp(pid, roster->pool[i], 32) == 0) {
            return roster->pool[i];
        }

        i = (i + OPEN_ADDR_JUMP) % roster->size;
    } while (i != hashval);

    return NULL;
}

struct RosterEntry *roster_remove(struct Roster *roster, const uint8_t pid[32]) {
    uint64_t hashval = _roster_hash_entry(roster, pid);
    uint32_t i = hashval;
    struct RosterEntry *hit;

    do {
        if (roster->pool[i] == NULL) {
            return NULL;
        }
        if (roster->pool[i] != (void *) 1 && memcmp(pid, roster->pool[i]->pid, 32) == 0) {
            hit = roster->pool[i];
            roster->pool[i] = (void *) 1;
            roster->length--;
            return hit;
        }

        i = (i + OPEN_ADDR_JUMP) % roster->size;
    } while (i != hashval);

    return NULL;
}

struct Roster *fetch_roster(MYSQL *connection) {
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    uint8_t id[32] = { 0xCC };
    uint64_t id_len = 0;
    uint64_t aid = 0;
    int res;
    struct Roster *roster_map = roster_create();

    if (roster_map == NULL) {
        ERROR("Failed to create roster structure\n");
        return NULL;
    }

    static const char *query = "SELECT PID, AID FROM roster";

    memset(bind, 0, sizeof(bind));

    stmt = mysql_stmt_init(connection);
    if (stmt == NULL) {
        ERROR("Failed to prepare roster stmt, out of memory\n");
        return NULL;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        ERRORF("Failed to prepare roster stmt, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        roster_free(roster_map);
        return NULL;
    }

    bind[0].buffer_type = MYSQL_TYPE_BLOB;
    bind[0].buffer = id;
    bind[0].buffer_length = 32;
    bind[0].length = &id_len;

    bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[1].buffer = &aid;

    if (mysql_stmt_bind_result(stmt, bind)) {
        ERRORF("Failed to bind results, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        roster_free(roster_map);
        return NULL;
    }

    if (mysql_stmt_execute(stmt)) {
        ERRORF("Failed to execute roster query, error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        roster_free(roster_map);
        return NULL;
    }
    
    while ((res = mysql_stmt_fetch(stmt)) == 0) {
        roster_insert(roster_map, roster_create_entry(id, aid));
    }

    return roster_map;
}
