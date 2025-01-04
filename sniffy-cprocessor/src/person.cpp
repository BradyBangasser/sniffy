#include "person.hpp"

Person::Person() {

}

uint32_t Person::parse_height(const char *str) {
    uint8_t feet;
    uint8_t inches;
    uint8_t total_inches;
    const char *curs;
    char nums[4] = { 0 };
    uint8_t n_i = 0;

    curs = str;

    while (curs != NULL) {
        if (*curs >= 0x30 && *curs < 0x3A) {
            nums[n_i++] = *curs;
            if (n_i == sizeof(nums) - 1) break;
        }
        curs++;
    }

    if (!n_i) return 0;

    if (n_i == 1) {
        // assume feet
        return (nums[0] - 0x30) * 12;
    } else if (n_i == 2) { 
        // assume inches
        return atoi(nums);
    } else if (n_i == 3) {
        if ((nums[0] - 0x30) >= 4) {
            // assume feet/inches
            return (nums[0] - 0x30) * 12 + atoi(nums + 1);
        } else if ((nums[0] - 0x30) == 0) {
            // inches again
        return atoi(nums);
        } else {
            // whatever the metric system is
            return atoi(nums) / 2.54f;
        }
    } else {
        return 0;
    }
}
