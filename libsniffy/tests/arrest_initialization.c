#include "arrest.h"

int main() {
    sniff_arrest arr0;
    sniff_arrest arr1;

    memset(&arr1, 0, sizeof(arr1));

    if (sniff_arrest_init(&arr0)) return 1;

    if (memcmp(&arr0, &arr1, sizeof(arr1))) return 2;

    if (sniff_arrest_destroy(&arr0)) return 3;

    return 0;
}
