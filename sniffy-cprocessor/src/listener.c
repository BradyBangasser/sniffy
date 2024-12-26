#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/sniffy.socket"

void slisten() {
    struct sockaddr_un addr;
    int sock, result, asock;
    const int enable = 1;

    if (!access(SOCKET_PATH, F_OK)) {
        printf("Removing file\n");
        remove(SOCKET_PATH);
    }

    sock = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Socket creation failed, errno %d\n", errno);
        return;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        printf("Error setting socket option, errno: %d\n", errno);
        return;
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, SOCKET_PATH);

    result = bind(sock, (struct sockaddr *) &addr, sizeof(addr));
    if (result) {
        printf("Bind failed, errno: %d\n", errno);
        return;
    }

    result = listen(sock, 5);
    if (result) {
        printf("Listen failed, errno: %d\n", errno);
        return;
    }

    while (1) {
        asock = accept(sock, NULL, NULL);
        if (asock == -1) {
            printf("ERROR: %d\n", errno);
            break;
        }

        printf("Accepted socket connection: %d\n", asock);
    }
}
