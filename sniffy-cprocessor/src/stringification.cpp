#include "stringification.hpp"

char *stringification::capitialize_name(char *name) {
    static constexpr char capitialize_chars[] = { '\'', ' ', '\n', '\t' };
    *name = toupper(*name);
    char *curs = name + 1;

    while (*curs) {
        for (int i = 0; i < sizeof(capitialize_chars); i++) {
            if (*(curs - 1) == capitialize_chars[i]) {
                *curs = toupper(*curs);
                break;
            }
        }

        curs++;
    }
    return name;
}
