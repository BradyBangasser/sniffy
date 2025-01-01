#pragma once

#include "charge.hpp"
#include "stringification.hpp"
#include "person.hpp"

#include <rapidjson/document.h>
#include <vector>
#include <iostream>

class Arrest {
    friend class Person;
    friend class Charge;
    private:
        // Counter for aids
        static uint64_t id_c;
        uint8_t pid[32];
        uint64_t id;

        Person *person;
        std::vector<std::string> notes;

        std::vector<Charge> charges;
        const char docket_number[16];

        Arrest();

        template <typename E, typename A> static bool process_json_field(Arrest &arr, std::string field, rapidjson::GenericValue<E, A> &val) {
            std::cout << field << std::endl;

            if (field == "FirstName") return arr.person->set_first_name(val);
            if (field == "MiddleName") return arr.person->set_middle_name(val);
            if (field == "LastName") return arr.person->set_last_name(val);
            if (field == "Suffix") return arr.person->set_suffix(val);
            if (field == "Charges") return !!(arr.charges = Charge::vec_from_json(val.GetArray()));
            
            if (!val.IsNull()) {
                if (val.IsArray()) {
                    std::string str_arr = stringification::json_array_to_string(val.GetArray());
                    if (str_arr.length() > 0) arr.notes.push_back(str_arr);
                } else if (val.IsString()) {
                    if (val.GetStringLength() > 0) arr.notes.push_back(val.GetString());
                }
            }
            return true;
        }
    public:
        template <bool C, typename T> static Arrest from_json(rapidjson::GenericObject<C, T> obj) {
            Arrest arr;
            rapidjson::GenericMemberIterator curs = obj.MemberBegin();

            while (curs != obj.MemberEnd()) {
                process_json_field(arr, curs->name.GetString(), curs->value);
                curs++;
            }
        }


        void recompute_id();
        ~Arrest();
};
