#include "listener.h"
#include "database.h"

int main() {
    if (!database::connect_db()) {
        return -1;
    }

    slisten();
    return 0;
}
