#include "processor.hpp"
#include <rapidjson/writer.h>
#include <iostream>

#include "arrest.hpp"

void Processor::process_json_string(const char *str) {
    std::cout << strlen(str) << std::endl;
    Processor::process_json_string(rapidjson::StringStream(str));
}

static const char* kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };

void Processor::process_json_string(rapidjson::StringStream str) {
    rapidjson::Document doc;
    doc.ParseStream(str);

    if (doc.IsNull() || (!doc.IsObject() && !doc.IsArray())) {
        std::cout << "TYPE ERROR" << std::endl;
        std::cout << kTypeNames[doc.GetType()] << std::endl;
        return;
    }

    rapidjson::GenericArray inmates = doc.IsObject() ? doc.GetObject().FindMember("Inmates")->value.GetArray() : doc.GetArray();

    std::cout << inmates.Size() << std::endl;

    for (auto &inmate : inmates) {
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        inmate.Accept(writer);

        Arrest::from_json(inmate.GetObject());
    }
}
