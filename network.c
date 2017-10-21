#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>

// Given address and port, create the connection and return the socket file descriptor
int create_connection(const char* address, const char* port, int socktype, int protocol)
{
    struct addrinfo hint;
    struct addrinfo *result;

    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = socktype;
    hint.ai_protocol = protocol;

    int addinfo = getaddrinfo(address, port, &hint, &result);
    if (addinfo != 0) {
        fprintf(stderr, "%s\n", gai_strerror(addinfo));
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sockfd == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, result->ai_addr, result->ai_addrlen) != 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);

    return sockfd;
}

// Bind socket to port
int create_bind(const char* address, const char* port, int socktype, int protocol)
{
    struct addrinfo hint;
    struct addrinfo *result;

    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = socktype;
    hint.ai_protocol = protocol;

    int addinfo = getaddrinfo(address, port, &hint, &result);
    if (addinfo != 0) {
        fprintf(stderr, "%s\n", gai_strerror(addinfo));
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sockfd == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, result->ai_addr, result->ai_addrlen) != 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);

    return sockfd;
}
