#ifndef NETWORK_H
#define NETWORK_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX_FILENAME_LENGTH 245
#define INIT 0x00
#define DATA 0x01
#define ACK 0x11
#define MAX_SEGMENT_SIZE 1500

struct __attribute__((__packed__)) init {
    char filename[MAX_FILENAME_LENGTH];
    unsigned long size;
    int mode;
    unsigned long seq_num;
};

struct __attribute__((__packed__)) data {
    unsigned long seq_num;
    int size;
    char bytes[];
};

struct __attribute__((__packed__)) ack {
    unsigned long seq_num;
};

union payload {
    struct init init;
    struct data data;
    struct ack ack;
};

struct __attribute__((packed)) packet {
    char type;
    union payload payload;
};

// Given address and port, create the connection and return the socket file descriptor
int create_connection(const char* address, const char* port, int socktype, int protocol);

// bind the given socket
int create_bind(const char* address, const char* port, int socktype, int protocol);

#endif
