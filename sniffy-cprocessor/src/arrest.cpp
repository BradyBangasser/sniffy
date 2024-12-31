#include "arrest.hpp"

template <typename T> Arrest Arrest::from_json(rapidjson::GenericObject<true, T> obj) {
    Arrest arr;
    
    rapidjson::GenericMember curs = obj.begin();
    while (curs != obj.end()) {
        switch (curs.name) {

        }
    }


}
