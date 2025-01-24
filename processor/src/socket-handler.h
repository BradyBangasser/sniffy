#pragma once

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

    void *socket_handler(void *fd);

    struct RosterData {
        char state_code[2];
        uint32_t facId;
        uint32_t data_len;
        uint8_t unused[20];
    }__attribute__((packed));

#ifdef __cplusplus
}
#endif
