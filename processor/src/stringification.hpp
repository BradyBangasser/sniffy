#pragma once

#include <rapidjson/document.h>
#include <algorithm>

namespace stringification {
    template <bool C, typename A> std::string json_array_to_string(rapidjson::GenericArray<C, A> arr) {
        std::string str = "";
        rapidjson::Value::ConstValueIterator curs = arr.Begin();

        while (curs != arr.End()) {
            if (curs != arr.Begin()) {
                str += ", ";
            }
            str += curs->GetString();
            curs++;
        }

        return str;
    }

    char *capitialize_name(char *name);
    std::string capitialize_name(std::string name);

    static inline std::string lower_str(std::string str) {
        std::string s = str;
        std::transform(s.begin(), s.end(), s.begin(), [](uint8_t c) { return tolower(c); });
        return s;
    }
}
