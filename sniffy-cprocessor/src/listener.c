#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <string.h>

void slisten() {
    struct sockaddr_un addr;
    int sock, result;

    sock = socket(AF_LOCAL, SOCK_STREAM, IPPROTO_RAW);    
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, "/tmp/sniffy.socket");

    result = bind(sock, (struct sockaddr *) &addr, sizeof(addr));

    result = listen(sock, 5);

    while (true) {
        accept
}
