#pragma once

#include "charge.hpp"

#include <rapidjson/document.h>
#include <vector>
#include <chrono>

class Arrest {
    private:
        uint8_t id[32];
        std::vector<Charge> charges;
        std::time_t timestamp;
        std::time_t released;
        const char docket_number[16];

        Arrest();
    public:
        template <typename T> static Arrest from_json(rapidjson::GenericObject<true, T> obj);

};
