#include <string.h>
#include <sys/socket.h>
#include <string>
#include "processor.hpp"

#include "logging.h"

extern "C" void *socket_handler(void *fdp) {
    if (fdp == NULL) {
        return NULL;
    }

    int32_t initial_size;
    ssize_t res;
    uint8_t *buffer;
    int fd = *((int *) fdp);
    std::string json;

    DEBUGF("Attempting to receive data from socket %d\n", fd);
    res = recv(fd, &initial_size, sizeof(initial_size), 0);

    if (res == -1) {
        ERRORF("Failed to receive data from socket, errno: %d\n", errno);
        return NULL;
    }

    initial_size += 4;

    buffer = (uint8_t *) malloc(sizeof(uint8_t) * (initial_size + 1));
    if (buffer == NULL) {
        ERRORF("Failed to allocate memory, errno: %d\n", errno);
        return NULL;
    }

    memset(buffer, 0, initial_size + 1);

    res = recv(fd, buffer, (initial_size + 1), 0);
    
    if (res == -1) {
        ERRORF("Failed to receive data from socket, errno: %d\n", errno);
        return NULL;
    }

    json = (char *) buffer + 4;
    Processor::process_json_string((char *) buffer + 4);

    free(buffer);
    return NULL;
}
