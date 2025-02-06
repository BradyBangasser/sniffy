#include "arrest.h"

int main() {
    Arrest arr0;
    Arrest arr1;

    memset(&arr1, 0, sizeof(arr1));

    if (arrest_init(&arr0)) return 1;

    if (memcmp(&arr0, &arr1, sizeof(arr1))) return 2;

    if (arrest_destroy(&arr0)) return 3;

    return 0;
}
