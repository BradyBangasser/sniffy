#pragma once

#include <string>
#include <rapidjson/document.h>

class Processor {
    public:
        static void process_json_string(rapidjson::StringStream str);
        static void process_json_string(const char *str);
};
