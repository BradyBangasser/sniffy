#include <string.h>
#include <sys/socket.h>
#include <cinttypes>
#include <string>
#include <iostream>
#include "processor.hpp"

extern "C" void *socket_handler(void *fdp) {
    if (fdp == NULL) {
        return NULL;
    }

    int32_t initial_size;
    ssize_t res;
    uint8_t *buffer;
    int fd = *((int *) fdp);
    std::string json;

    res = recv(fd, &initial_size, sizeof(initial_size), 0);
    std::cout << initial_size << std::endl;
    initial_size += 4;

    buffer = (uint8_t *) malloc(sizeof(uint8_t) * (initial_size + 1));
    if (buffer == NULL) {
        return NULL;
    }

    memset(buffer, 0, initial_size + 1);

    res = recv(fd, buffer, (initial_size + 1), 0);
    std::cout << buffer + 4 << std::endl;
    json = (char *) buffer + 4;

    // for (int i = 0; i < res + 10; i++) {
    //     std::cout << buffer[i];
    // }
    // std::cout << std::endl;

    Processor::process_json_string((char *) buffer + 4);
    std::cout << "HERER" << std::endl;

    free(buffer);
    return NULL;
}
