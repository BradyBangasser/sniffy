#include "listener.h"
#include "database.h"

#include <unistd.h>

int main() {
    atexit(database::disconnect_db);
    if (!database::connect_db()) {
        return -1;
    }

    slisten();
    return 0;
}
