#pragma once
#include <string>
#include <cinttypes>
#include <vector>
#include <rapidjson/document.h>

#include "arrest.hpp"

class Person {
    friend class Arrest;
    private:
        std::string first_name;
        std::string middle_name;
        std::string last_name;
        std::string *suffix;
        std::vector<std::string> notes;
        std::string versioning;
        uint8_t id[32];
        
        Person();
    public:
        Person(uint8_t id[32]);

        inline bool set_first_name(const std::string &str) {
            if (str.length() <= 0) {
                return false;
            }

            first_name = str;
            return true;
        }

        template <typename E, typename A> inline bool set_first_name(rapidjson::GenericValue<E, A> &val) {
            if (val.IsString()) return set_first_name(val.GetString());
            return false;
        }

        inline bool set_middle_name(const std::string &str) {
            if (str.length() <= 0) {
                return false;
            }

            middle_name = str;
            return true;
        }

        template <typename E, typename A> inline bool set_middle_name(rapidjson::GenericValue<E, A> &val) {
            if (val.IsString()) return set_middle_name(val.GetString());
            return false;
        }

        inline bool set_last_name(const std::string &str) {
            if (str.length() <= 0) {
                return false;
            }

            last_name = str;
            return true;
        }

        template <typename E, typename A> inline bool set_last_name(rapidjson::GenericValue<E, A> &val) {
            if (val.IsString()) return set_last_name(val.GetString());
            return false;
        }

        inline bool set_suffix(const std::string *const str) {
            if (str == NULL) {
                suffix = NULL;
            }

            suffix = new std::string(str->c_str());
            return true;
        }

        template <typename E, typename A> inline bool set_suffix(rapidjson::GenericValue<E, A> &val) { return !!(suffix = val.IsString() ? suffix = new std::string(val.GetString()) : suffix = NULL); }

        inline bool add_note(std::string str) {
            notes.push_back(str);
            return true;
        }
};
