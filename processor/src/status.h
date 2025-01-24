#pragma once

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif
#define FOREACH_STATUS_STATE(STATE) STATE(INITIALIZING) \
    STATE(PROCESSING) \
    STATE(UPDATING_DATABASE) \
    STATE(INSERTING_INTO_DATABASE) \
    STATE(ERROR) \
    STATE(CACHE_HIT) \
    STATE(KILLING_SELF)
#define ENUM_STATUS_STATE(ENUM) ENUM,
#define STRING_STATUS_STATE(STRING) #STRING,
    enum StatusState {
        FOREACH_STATUS_STATE(ENUM_STATUS_STATE)
    };

    const static char *STATUS_STATE_STRINGS[] = { FOREACH_STATUS_STATE(STRING_STATUS_STATE) };

    typedef struct {
        uint32_t expected;
        uint32_t current;
        enum StatusState state;

    } Status;

    struct _StatusNode {
        Status s;
        struct _StatusNode *next;
        struct _StatusNode *prev;
    };

    Status *status_state_create(uint32_t expected, enum StatusState initial_state);
    void status_state_destroy(Status *status);
    void update_state();
#ifdef __cplusplus
}
#endif
