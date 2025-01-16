#include "stringification.hpp"

static inline char capitialize_char(char p, char c) {
    static constexpr char capitialize_chars[] = { '\'', ' ', '\n', '\t', '-', '/' };

    if (p == 0x0) return toupper(c);
    if (c == '`') return '\'';

    for (int i = 0; i < sizeof(capitialize_chars); i++) {
        if (p == capitialize_chars[i]) {
            return toupper(c);
        }
    }

    return c;
}

char *stringification::capitialize_name(char *name) {
    *name = capitialize_char(0x0, *name);
    char *curs = name + 1;

    while (*curs) {
        *curs = capitialize_char(*(curs - 1), *curs);
        curs++;
    }
    return name;
}

std::string stringification::capitialize_name(std::string name) {
    char *arr = new char[name.length() + 1];
    memset(arr, 0, name.length() + 1);
    strncpy(arr, name.c_str(), name.length());
    stringification::capitialize_name(arr);
    std::string new_str = arr;
    delete[] arr;
    return new_str;
}
