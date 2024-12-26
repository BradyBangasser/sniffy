#pragma once
#include <string>
#include <cinttypes>

class Person {
    public:
        Person() = delete;

        Person(uint8_t id[32]);
    private:
        std::string first;
        std::string middle;
        std::string last;
        std::string notes;
        std::string versioning;
        uint8_t id[32];

};
