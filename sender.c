#include <poll.h>
#include <argp.h>
#include <stdlib.h>
#include "network.h"


// Construct and send init packet
int send_init(int sockfd, const char filename[MAX_FILENAME_LENGTH],
              unsigned long size, int mode, unsigned long seq_num) {
    struct init init;
    struct packet packet;
    memset(&init, 0, sizeof(struct init));
    memset(&packet, 0, sizeof(struct packet));

    strncpy(init.filename, filename, MAX_FILENAME_LENGTH);
    init.mode = mode;
    init.size = size;
    init.seq_num = seq_num;

    packet.type = INIT;
    packet.payload.init = init;

    printf("Packet Type: %x\nFilename: %s\nFile size: %ld\n",
            packet.type, packet.payload.init.filename,
            packet.payload.init.size);

    return send(sockfd, &packet, sizeof(packet), 0);
}

// print packet
void print_packet(const void *packet, int size) {
    char* bytes = (char*) packet;

    printf("Total size: %d\n", size);

    int written = 0, last = 0;
    for (int i = 0; i < size; i++) {
        unsigned char uc = bytes[i];
        if (i % 16 == 0)
            printf("0x%04x:   ", i);
        written += printf("%02x", uc);
        if ((i + 1) % 2 == 0)
            written += printf(" ");
        if ((i + 1) % 16 == 0 || i == size-1) {
            if (i == size - 1)
                 last = i + 1 - (i + 1) % 16;
            else
                last = i - 15;
            for (int j = written; j < 40; j++)
                printf(" ");
            written = 0;
            printf("\t");
            for (int j = last; j <= i; j++) {
                if (bytes[j] < 32)
                    printf(".");
                else
                    printf("%c", bytes[j]);
            }
            printf("\n");
        }
    }
    printf("\n\n");
}

// Construct and send data packet
int send_data(int sockfd, long seq_num, int size, char* bytes) {
    struct data* data = malloc(sizeof(struct data) + size);
    struct packet* packet = malloc(sizeof(struct packet) + size);
    memset(data, 0, sizeof(struct data) + size);
    memset(packet, 0, sizeof(struct packet) + size);

    memcpy(&data->bytes, bytes, size);
    data->size = size;
    data->seq_num = seq_num;

    packet->type = DATA;
    memcpy(&packet->payload, data, sizeof(struct data) + size);

    int sizesent = send(sockfd, packet, sizeof(struct packet) + size, 0);
    print_packet(packet, data_packet_size);
    if (sizesent == -1) {
        fprintf(stderr, "Weird error\n");
    }

    free(data);
    free(packet);

    return sizesent;
}

// Sender implementation
void start_sender(int mode, const char* port, const char* hostname, const char filename[256]) {
    int sockfd = create_connection(hostname, port, SOCK_DGRAM, IPPROTO_UDP);

    printf("Mode: %d\nPort: %s\nHostname: %s\nFilename: %s\n",
           mode, port, hostname, filename);

    // fake file
    int size = 1224 * 5;
    char file[size];
    for (int i = 0; i < size; i++) {
        file[i] = 'A';
    }
    // offset of file
    int offset = 0;
    // sequence number
    unsigned long seq_num = 0;
    // sequence number base
    unsigned long seq_base = 0;
    // sequence number max
    unsigned long seq_max = mode;

    struct pollfd fds;

    fds.fd = sockfd;
    fds.events = POLLIN;

    send_init(sockfd, filename, size, mode, seq_num);

    // create packet struct on the heap
    struct packet* packet = malloc(MAX_SEGMENT_SIZE);
    memset(packet, 0, MAX_SEGMENT_SIZE);

    while (offset < size) {
        // Wait for ack here
        memset(packet, 0, MAX_SEGMENT_SIZE);
        // TODO don't block here
        // We need to time out if last_ack
        // hasn't been updated for a while
        int ret = poll(&fds, 1, 500); // TODO Change to define
        if (ret == -1) {
            fprintf(stderr, "Weird polling error\n");
        } else if (ret == 0) {
            printf("Took longer than 500ms\n");
            // TODO resend here
            // Also keep track of how many times we had to resend
        } else {
            recv(sockfd, packet, MAX_SEGMENT_SIZE, 0);
            switch (packet->type) {
                case ACK:
                    seq_max = (seq_max - seq_base) + packet->payload.ack.seq_num;
                    seq_base = packet->payload.ack.seq_num;
                    seq_num = seq_base;
                    printf("Received ack: %lu\n", seq_base);
                    break;
                default:
                    fprintf(stderr, "Received unknown packet");
            }
        }

        printf("seq_base = %ld seq_num = %ld seq_max = %ld\n", seq_base, seq_num, seq_max);

        if (seq_base <= seq_num && seq_num <= seq_max) {
            // Calculate how much data should be sent
            int send_size = MAX_SEGMENT_SIZE - sizeof(struct packet);
            offset = (send_size < size - offset) ? seq_num * send_size : size;
            int sent = 0;
            printf("Sending %d data\n", send_size);
            if ((sent = send_data(sockfd, seq_num++, send_size, file + offset)) <= 0) {
                fprintf(stderr, "Something went wrong went sending");
            }
            printf("Sent %d bytes with seq: %lu\n", sent, seq_num);
        }
    }
}

struct arguments
{
    int mode;
    char* port;
    char* hostname;
    char filename[255];
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
    struct arguments* arguments = state->input;
    switch (key) {
        case 'm':
            arguments->mode = atoi(arg);
            break;
        case 'p':
            arguments->port = arg;
            break;
        case 'h':
            arguments->hostname = arg;
            break;
        case 'f':
            strncpy(arguments->filename, arg, 255);
            break;
        //case ARGP_KEY_END:
        //    argp_usage(state);
        //    break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    struct arguments arguments;
    memset(&arguments, 0, sizeof(arguments));
    arguments.mode = 0;
    arguments.port = "15180";
    arguments.hostname = "127.0.0.1";
    memset(arguments.filename, 0, 255);

    static struct argp_option options[] = {
        {"mode", 'm', "MODE", 0, "Mode", 0},
        {"port", 'p', "PORT", 0, "Port number", 0},
        {"hostname", 'h', "HOSTNAME", 0, "Hostname", 0},
        {"filename", 'f', "FILENAME", 0, "File name", 0},
        {0}
    };

    struct argp argp = { options, parse_opt, "", 0, 0, 0, 0};
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    start_sender(arguments.mode, arguments.port, arguments.hostname, arguments.filename);

    return 0;
}
