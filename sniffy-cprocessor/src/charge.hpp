#pragma once

#include <string>
#include <ctype.h>
#include <cinttypes>
#include <iostream>
#include <vector>
#include <rapidjson/document.h>
#include <algorithm>

class Charge {
    private:
        uint32_t bond;
        uint64_t id;
        uint64_t arrest_id;
        std::string bond_status;
        std::string bond_type;
        std::string timestamp;
        std::string statute_id;
        std::string statute;
        std::string docket_id;
        std::vector<std::string> notes;
        std::string insert_version;
        std::string update_version;
    public:
        inline std::string get_sid() { return statute_id; }
        inline std::vector<std::string> get_notes() { return notes; }
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
                    c.statute = curs->name.GetString();
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
                        std::cout << "NOT UINT64: " << curs->value.GetType() << " " << curs->value.IsInt() << std::endl;
                        return false;
                    }
                }

                if (field == "docketnumber") {
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

                if (curs->value.IsString()) {
                    c.notes.push_back(curs->value.GetString());
                } else {
                    std::cout << "TYPE: " << curs->value.GetType() << std::endl;
                }
            }

            charge = c;
            return true;
        }
};
