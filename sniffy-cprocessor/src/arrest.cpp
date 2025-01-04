#include "arrest.hpp"
#include "logging.h"

uint64_t Arrest::id_c = 0;

Arrest::Arrest() : id{ Arrest::id_c++ }, docket_number{ 0 } {
    DEBUGF("Creating new arrest, id: %016lu\n", this->id);
    person = new Person();
}

Arrest::~Arrest() {
    delete person;
}
