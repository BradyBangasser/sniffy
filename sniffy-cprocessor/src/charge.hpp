#pragma once

#include <string>
#include <ctype.h>
#include <vector>
#include <rapidjson/document.h>
#include <algorithm>
#include <mysql/mysql.h>

#include "logging.h"
#include "stringification.hpp"

class Charge {
    private:
        uint32_t bond;
        uint64_t id;
        uint64_t arrest_id;
        static uint32_t id_c;
        std::string bond_status;
        std::string bond_type;
        std::string statute_id;
        std::string statute;
        std::string docket_id;
        struct tm charged_at;
        std::vector<std::string> notes;
        std::string insert_version;
        std::string update_version;

        Charge(uint64_t id, uint64_t aid, uint32_t bond, std::string sid, std::string docket, std::vector<std::string> notes);
    public:
        Charge();
        inline std::string get_sid() const { return statute_id; }
        inline std::vector<std::string> get_notes() const { return notes; }
        template <bool C, typename A> static std::vector<Charge> vec_from_json(uint64_t aid, rapidjson::GenericArray<C, A> charges) {
            std::vector<Charge> parsed_charges;

            rapidjson::Value::ConstValueIterator curs = charges.Begin();

            while (curs != charges.End()) {
                Charge c;
                if (from_json(c, aid, curs->GetObject())) {
                    parsed_charges.push_back(c);
                }
                curs++;
            }

            return parsed_charges;
        }

        template <bool C, typename A> static bool from_json(Charge &charge, uint64_t aid, rapidjson::GenericObject<C, A> json) {
            rapidjson::GenericMemberIterator curs = json.MemberBegin();

            Charge c;

            c.arrest_id = aid;

            for (;curs != json.MemberEnd(); curs++) {
                std::string field = curs->name.GetString();
                if (field == "name") {
                    std::string val = std::string(curs->value.GetString());
                    std::transform(val.begin(), val.end(), val.begin(), ::toupper);
                    c.statute_id = std::string("IA-") + val;
                    continue;
                }

                if (field == "description") {
                    c.statute = stringification::capitialize_name(curs->value.GetString());
                    continue;
                }

                if (field == "bondamount") {
                    if (curs->value.IsString()) {
                        c.bond = atoi(curs->value.GetString());
                    } else if (curs->value.IsUint()) {
                        c.bond = curs->value.GetUint();
                    } else if (curs->value.IsDouble()) {
                        double b = curs->value.GetDouble();
                        c.bond = static_cast<uint32_t>(b);
                    } else {
                        return false;
                    }
                }

                if (field == "docketnumber" && curs->value.GetStringLength() > 0) {
                    std::string val = std::string(curs->value.GetString());
                    std::transform(val.begin(), val.end(), val.begin(), ::toupper);
                    c.docket_id = std::string("IA-") + val;
                    continue;
                }

                if (field == "bondtype") {
                    c.bond_type = curs->value.GetString();
                    continue;
                }
                
                if (field == "bondstatus") {
                    std::string val = std::string(curs->value.GetString());
                    std::transform(val.begin(), val.end(), val.begin(), ::toupper);
                    c.bond_status = val;
                    continue;
                }

                if (field == "date") {
                    if (strptime(curs->value.GetString(), "%Ft%T", &c.charged_at)) {
                        c.generate_id();
                        continue;
                    }

                    ERRORF("Failed to parse datetime '%s'\n", curs->value.GetString());
                    continue;
                }

                if (curs->value.IsString()) {
                    if (curs->value.GetStringLength() <= 0) continue;
                    std::string key = curs->name.GetString();
                    c.notes.push_back(key + ": " + curs->value.GetString());
                }
            }

            charge = c;
            return true;
        }

        bool verify();
        bool generate_id();
        bool upsert(MYSQL *connection);
        inline uint32_t get_bond() const { return bond; }
        inline void set_bond(uint32_t bond) { this->bond = bond; }
        inline uint64_t get_id() const { return id; }
        inline void set_id(uint64_t id) { this->id = id; }
        inline void set_aid(uint64_t aid) { this->arrest_id = aid; }
        inline void add_note(std::string note) { notes.push_back(note); }
        inline std::string get_docket_id() { return docket_id; }
        static std::vector<Charge> fetch(uint64_t aid, MYSQL *connection);
};
