#pragma once

#include <rapidjson/document.h>

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
}
