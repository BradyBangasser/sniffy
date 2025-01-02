#include "arrest.hpp"

uint64_t Arrest::id_c = 0;

Arrest::Arrest() : docket_number{ 0 }, id{ Arrest::id_c++ } {
    person = new Person();
}

Arrest::~Arrest() {
    delete person;
}
