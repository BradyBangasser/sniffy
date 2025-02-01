#include <string.h>
#include <sys/socket.h>
#include <string>
#include <poll.h>

#include "socket-handler.h"
#include "processor.hpp"
#include "logging.h"
#include "status.h"
#include "global.h"

extern "C" void *socket_handler(void *fdp) {
    if (fdp == NULL) {
        return NULL;
    }

    struct RosterData d;
    struct pollfd ps;

    uint8_t *buffer;
    int fd = *((int *) fdp), p;

    constexpr uint8_t ack = 0x06;

    std::string json;

    ps.fd = fd;
    ps.events |= POLLHUP;
    ps.revents |= POLLHUP;

    while ((p = poll(&ps, 1, 10)) != -1 && !(ps.revents & POLLHUP)) {
        Status *status = status_state_create(100, INITIALIZING);
        memset(&d, 0, sizeof(struct RosterData));

        if (recv(fd, (char *) &d, sizeof(struct RosterData), 0) < 0) {
            ERRORF("Error receiving data from socket (fd: %d), errno: %d\n", fd, errno);
            return NULL;
        }

        if ((d.state_code[0] - 0x41) > 0x5A || (d.state_code[1] - 0x41) > 0x5A) { 
            ERRORF("'%x%x' is an invalid state code\n", d.state_code[0], d.state_code[1]);
            return NULL;
        }

        if (d.data_len < 1) {
            WARNF("Received no data from Facility %c%c-%X\n", d.state_code[0], d.state_code[1], d.facId);
            return NULL;
        }

        send(fd, &ack, 1, 0);

        buffer = (uint8_t *) calloc(d.data_len + 1, 1);

        if (buffer == NULL) {
            ERROR("Failed to allocate memory for socket messages\n");
            return NULL;
        }

        if (recv(fd, (char *) buffer, d.data_len, 0) < d.data_len) {
            ERRORF("Error receiving data from socket (fd: %d), errno: %d\n", fd, errno);
            return NULL;
        }

        Processor::process_json_string(&d, (char *) buffer, status);

        free(buffer);
    }

    return NULL;
}
