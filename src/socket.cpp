#include "socket.h"
#include "logger.h"
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

Socket::Socket() : sock(-1) {}

Socket::~Socket() {
    if (sock >= 0) {
        ::shutdown(sock, SHUT_RDWR);
        ::close(sock);
    }
}

bool Socket::init(Type type) {
    if (type == TCP) {
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    } else {
        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    return sock >= 0;
}

bool Socket::close() {
    if (sock >= 0) {
        int result = ::close(sock);
        sock       = -1;
        return result == 0;
    }
    return true;
}

bool Client::sendall(const void *data, size_t length) {
    size_t sent = 0;
    while (sent < length) {
        int num = send(sock, data, length - sent, 0);
        if (num < 0) {
            close();
            return false;
        }

        sent += num;
        data = (const char *) data + num;
    }
    return true;
}

bool Client::recvall(void *data, size_t length) {
    size_t received = 0;
    while (received < length) {
        int num = recv(sock, data, length - received, 0);
        if (num <= 0) {
            close();
            return false;
        }

        received += num;
        data = (char *) data + num;
    }
    return true;
}

struct sockaddr_kinnay {
    uint16_t family;
    uint16_t port;
    uint32_t addr;
    char zero[8];
};

bool Server::bind(int port) {
    uint32_t reuseaddr = 1;
    int result         = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, 4);
    if (result < 0) {
        close();
        return false;
    }


    /*
    struct sockaddr_in bindAddress;
    memset(&bindAddress, 0, sizeof(bindAddress));
    bindAddress.sin_family      = AF_INET;
    bindAddress.sin_port        = htons(port);
    bindAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    result = ::bind(sock, (struct sockaddr *) &bindAddress, 16);*/

    sockaddr_kinnay serverAddr = {0};
    serverAddr.family   = AF_INET;
    serverAddr.port     = port;
    serverAddr.addr     = 0;

    result = ::bind(sock, (const struct sockaddr *) &serverAddr, 16);
    if (result < 0) {
        DEBUG_FUNCTION_LINE("Bind was not successful");
        close();
        return false;
    }
    DEBUG_FUNCTION_LINE("Bind was successful");
    return true;
}

bool Server::accept(Client *client) {
    int result = listen(sock, 1);
    if (result < 0) {
        DEBUG_FUNCTION_LINE("listen failed");
        close();
        return false;
    }

    int fd = ::accept(sock, 0, 0);
    if (fd < 0) {
        DEBUG_FUNCTION_LINE("accept failed");
        close();
        return false;
    }

    client->sock = fd;
    return true;
}
