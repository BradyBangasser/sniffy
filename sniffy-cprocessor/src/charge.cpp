#include "charge.hpp"
#include "logging.h"

#include <time.h>

uint32_t Charge::id_c = 0;

Charge::Charge() {
    generate_id();
    DEBUGF("Created charge %lx\n", id);
}

bool Charge::verify() {
    if (!id) return false;
    if (statute.length() < 1) return false;
    if (statute_id.length() < 1) return false;
    if (timestamp.length() < 1) return false;
    return true;
}

bool Charge::generate_id() {
    struct tm t;
    uint64_t buf;

    id = 0;

    if (!timelocal(&t)) {
        ERROR("Failed to get datetime\n");
        return false;
    }

    id = (t.tm_mon - 1);
    id <<= 60;
    buf = t.tm_mday--;
    id |= (buf << 55);
    buf = t.tm_year - 100;
    id |= (buf << 48);
    buf = t.tm_min;
    id |= (buf << 42);
    buf = t.tm_sec;
    id |= (buf << 37);
    buf = getpid() % (1 << 5);
    id |= (buf << 32);
    id |= id_c++;

    return true;
}
