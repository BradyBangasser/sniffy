#pragma once
#include <string>
#include <mysql/mysql.h>
#include <vector>
#include <rapidjson/document.h>

#include "global.h"
#include "stringification.hpp"
#include "sex.h"
#include "race.h"

class Person {
    friend class Arrest;
    private:
        std::string first_name;
        std::string middle_name;
        std::string last_name;
        std::string *suffix = 0;
        enum sex sex = S_UNKNOWN;
        enum race race = SR_UNKNOWN;
        uint16_t birth_year = 0;
        uint8_t height = 0; // in inches
        uint16_t weight = 0;
        std::vector<std::string> notes;
        std::string versioning;
        uint8_t id[32] = {0};
        bool id_set = false;
        
        Person();
    public:
        Person(uint8_t id[32]);

        static std::string id_to_str(uint8_t id[32]);
        inline std::string id_to_str() { return Person::id_to_str(this->id); }
        inline std::vector<std::string> get_notes() { return notes; }

        inline bool set_first_name(const std::string &str) {
            if (str.length() <= 0) {
                return false;
            }

            first_name = stringification::capitialize_name(str);
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

            middle_name = stringification::capitialize_name(str);

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

            last_name = stringification::capitialize_name(str);;
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

            suffix = new std::string(stringification::capitialize_name(*str));
            return true;
        }

        template <typename E, typename A> inline bool set_suffix(rapidjson::GenericValue<E, A> &val) { return !!(suffix = val.IsString() ? suffix = new std::string(val.GetString()) : suffix = NULL); }

        inline bool add_note(std::string str) {
            notes.push_back(str);
            return true;
        }

        inline void set_race(enum race race) {
            this->race = race;
        }
        inline void set_race(std::string race) { this->race = str_to_race(race.c_str()); }

        inline void set_sex(enum sex sex) {
            this->sex = sex;
        }

        inline void set_sex(std::string sex) {
            this->sex = str_to_sex(sex.c_str());
        }

        inline void set_birthyear(uint16_t birthyear) {
            this->birth_year = birthyear;
        }

        inline bool set_birthyear_by_age(uint8_t age) {
            if (age > 150) {
                // As of my writing of this software, the max theoretical age is roughly 125 with current technology, so 150 is the max age of a human
                return false;
            }

            // Get current year
            time_t now = time(NULL);
            tm *t = gmtime(&now);

            this->birth_year = t->tm_year + 1900 - age;
            char datestring[80] = { 0 };
            strftime((char *) datestring, sizeof(datestring), "Birth Year was calculated by age on %F at %H:%M ZULU", t);
            this->add_note(datestring);
            return true;
        }

        // returns 0 on failure
        static inline uint32_t parse_height(std::string str) { return parse_height(str.c_str()); }
        static uint32_t parse_height(const char *);

        inline bool set_height(const char *height) {
            this->height = parse_height(height);
            return height == 0;
        }

        inline bool set_weight(const char *str) {
            this->weight = atoi(str);
            return true;
        }

        inline bool set_weight(uint16_t weight) {
            this->weight = weight;
            return true;
        }

        inline const uint8_t *get_id() { return id; }

        inline std::string get_name() const { return first_name + " " + last_name; }

        // verify that this is in fact a valid person, will call generate_id if necessary and if genId is true
        bool verify(bool genId = true);
        bool generate_id();
        bool upsert(MYSQL *);
};
