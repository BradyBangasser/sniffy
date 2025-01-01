#pragma once

#include <string>
#include <cinttypes>
#include <vector>
#include <rapidjson/document.h>

class Charge {
    private:
        uint32_t bond;
        uint64_t id;
        uint64_t arrest_id;
        std::string bond_status;
        std::string timestamp;
        std::string statute_id;
        std::string statute;
        std::string docket_id;
        std::string notes;
        std::string insert_version;
        std::string update_version;
    public:
        template <bool C, typename A> static std::vector<Charge> vec_from_json(rapidjson::GenericArray<C, A> charges) {
            std::vector<Charge> parsed_charges;

            rapidjson::Value::ConstValueIterator curs = charges.Begin();

            while (curs != charges.End()) {
                Charge c;
                if (from_json(c, curs->GetObject())) {
                    parsed_charges.push_back(c);
                }
                curs++;
            }
        }

        template <bool C, typename A> static bool from_json(Charge &charge, rapidjson::GenericObject<C, A> json) {
            
        }
};
