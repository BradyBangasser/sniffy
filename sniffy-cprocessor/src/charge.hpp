#pragma once

#include <string>
#include <cinttypes>

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

};
