#pragma once

#include "charge.hpp"
#include "stringification.hpp"
#include "person.hpp"
#include "processor.hpp"
#include "processor.hpp"

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <mysql/mysql.h>
#include <vector>
#include <numeric>

class Arrest {
    private:
        // Counter for aids
        static uint16_t id_c;
        uint64_t id;
        uint32_t bond;
        uint32_t fac_id;
        char state_code[3] = {0};

        Person *person;
        uint8_t *pid;

        std::vector<std::string> notes;

        struct tm arrested_at;
        struct tm release_date;

        std::vector<Charge> charges;
        std::string booking_agency;

        Arrest();
        Arrest(uint64_t id, uint8_t pid[32], uint32_t bond, std::vector<std::string> notes);

        template <typename E, typename A> static bool process_json_field(Arrest &arr, std::string field, rapidjson::GenericValue<E, A> &val) {
            if (field == "firstname" || field == "fname") return arr.person->set_first_name(val);
            if (field == "middlename" || field == "mname") return arr.person->set_middle_name(val);
            if (field == "lastname" || field == "lname") return arr.person->set_last_name(val);
            if (field == "suffix") return arr.person->set_suffix(val);
            if (field == "race") { arr.person->set_race(val.GetString()); return true; }
            if (field == "sex") { arr.person->set_sex(val.GetString()); return true; }
            if (field == "primarycharge" || field == "primarychargedescription") return true; 
            if (field == "height") return arr.person->set_height(val.GetString());
            if (field == "charges") {
                arr.charges = Charge::vec_from_json(arr.id, arr.state_code, arr.arrested_at, val.GetArray());
                auto f = [&](const uint32_t &t, const Charge &c) { return t + c.get_bond(); };
                arr.bond = std::accumulate(arr.charges.begin(), arr.charges.end(), (uint32_t) 0, f);
                return true;
            }

            if (field == "bookingagency") {
                char *agency = new char[val.GetStringLength() + 1];
                agency[val.GetStringLength()] = 0;
                memcpy(agency, val.GetString(), val.GetStringLength());
                arr.booking_agency = stringification::capitialize_name(agency);
                delete[] agency;
                return true;
            }

            if (field == "age") {
                if (val.IsUint()) return arr.person->set_birthyear_by_age(val.GetUint());
                if (val.IsString()) return arr.person->set_birthyear_by_age(atoi(val.GetString()));
                return false;
            }

            if (field == "weight") {
                if (val.IsUint()) return arr.person->set_weight(val.GetUint());
                if (val.IsString()) return arr.person->set_weight(val.GetString());
                return false;
            }

            if (field == "birthyear") {
                arr.person->set_birthyear(val.GetUint());
                return true;
            }

            if (field == "arrestdate" || field == "arrest_date") {
                return strptime(val.GetString(), "%m/%d/%Y %r", &arr.arrested_at) || 
                    strptime(val.GetString(), "%m/%d/%Y", &arr.arrested_at);
            }

            if (field == "releasedate" || field == "release_date") {
                return strptime(val.GetString(), "%m/%d/%Y %r", &arr.release_date) ||
                    strptime(val.GetString(), "%m/%d/%Y", &arr.release_date);
            }

            if (!val.IsNull()) {
                if (val.IsArray()) {
                    std::string str_arr = stringification::json_array_to_string(val.GetArray());
                    if (str_arr.length() > 0) arr.notes.push_back(field + ": " + str_arr);
                } else if (val.IsString()) {
                    if (val.GetStringLength() > 0) arr.notes.push_back(field + ": " + val.GetString());
                }
            }

            return true;
        }
    public:
        template <bool C, typename T> static Arrest from_json(struct FacilityData &d, rapidjson::GenericObject<C, T> obj) {
            Arrest arr;
            rapidjson::GenericMemberIterator curs = obj.MemberBegin();

            arr.fac_id = d.id;
            memcpy(arr.state_code, d.state_code, 2);

            while (curs != obj.MemberEnd()) {
                process_json_field(arr, curs->name.GetString(), curs->value);
                curs++;
            }

            if (!arr.verify()) {
                rapidjson::StringBuffer sb;
                rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
                rapidjson::GenericValue sub(obj);
                sub.Accept(writer);
                DEBUGF("Failed to verify arrest (%s) -> %s\n", arr.get_person()->get_name().c_str(), sb.GetString());
                // throw std::exception();
            }

            return arr;
        }


        // ensures all required elements of the arrest exist
        bool verify();
        bool generate_id(struct FacilityData *fac_dat = NULL);
        bool upsert(MYSQL *);
        
        inline Person *get_person() { return person; }

        inline uint64_t get_id() { return id; }
        inline void set_id(uint64_t new_id) {
            id = new_id;
            for (Charge &c : this->charges) {
                c.set_aid(new_id);
            }
        }

        inline bool has_been_released() {
            return (release_date.tm_year != 0 && difftime(mktime(&release_date), time(NULL)) < 0);
        }

        inline std::vector<Charge> &get_charges() { return charges; }
        inline void swap_charges(std::vector<Charge> &charges) { std::swap(charges, this->charges); }

        static Arrest *fetch(uint64_t id, MYSQL *connection);

        ~Arrest();
};
