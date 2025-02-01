#pragma once

#include <rapidjson/document.h>

#include "socket-handler.h"
#include "status.h"

struct FacilityData {
    FacilityData(const struct RosterData *);
    const char state_code[3];
    uint32_t id;
    uint8_t flags;
};

class Processor {
    public:
        static void process_json_string(const struct RosterData *d, rapidjson::StringStream str, Status *status = NULL);
        static void process_json_string(const struct RosterData *d, const char *str, Status *status = NULL);
};
