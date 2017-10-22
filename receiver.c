#include <argp.h>
#include <sys/types.h>
#include <unistd.h>
#include "network.h"


int send_ack(int sockfd, const struct sockaddr* dest_addr,
             socklen_t addrlen, long seq_num) {
    struct ack ack;
    struct packet packet;
    memset(&ack, 0, sizeof(struct ack));
    memset(&packet, 0, sizeof(struct ack) + sizeof(char));

    ack.seq_num = seq_num;

    packet.type = ACK;
    packet.payload.ack = ack;

    return sendto(sockfd, &packet, sizeof(packet), 0,
                  dest_addr, addrlen);
}

// Receiver implementation
void start_receiver(int mode, const char* port, const char* hostname) {
    int sockfd = create_bind(hostname, port, SOCK_DGRAM, IPPROTO_UDP);

    struct packet* packet = malloc(MAX_SEGMENT_SIZE);
    memset(packet, 0, MAX_SEGMENT_SIZE);

    unsigned long next_seq_num = 0;

    // temps
    struct sockaddr src_addr;
    socklen_t addrlen = sizeof(src_addr);
    memset(&src_addr, 0, addrlen);

    // file
    FILE* file;
    long offset = 0;
    int bytes = 0;
    long max_data_size = MAX_SEGMENT_SIZE - sizeof(struct data) - sizeof(char);

    long size = 1;

    while (offset + bytes < size &&
           recvfrom(sockfd, packet, MAX_SEGMENT_SIZE, 0,
                    &src_addr, &addrlen)) {
        switch (packet->type) {
            case INIT:
                if (packet->payload.init.mode != mode) {
                    // exit on mode mismatch
                    fprintf(stderr, "Mode mismatch\n");
                    exit(EXIT_FAILURE);
                }
                char buffer[MAX_FILENAME_LENGTH + 10];
                snprintf(buffer, MAX_FILENAME_LENGTH + 10, "%s%d",
                         packet->payload.init.filename, getpid());
                printf("Created file %s\n", buffer);
                size = packet->payload.init.size;
                file = fopen(buffer, "w+");
                // TODO handle weird file errors
                next_seq_num = packet->payload.init.seq_num;
                // TODO refactor error handling into send_ack
                int send_size = send_ack(sockfd, &src_addr, addrlen, next_seq_num);
                if (send_size == -1) {
                    fprintf(stderr, "%s\n", strerror(errno));
                }
                break;
            case DATA:
                // only increase the sequence number if we got what we wanted
                if (packet->payload.data.seq_num == next_seq_num) {
                    printf("Received Data Seq: %ld\n", packet->payload.data.seq_num);
                    next_seq_num = packet->payload.data.seq_num + 1;
                    // Write to disk here
                    // TODO handle weird file errors
                    offset = packet->payload.data.seq_num * max_data_size;
                    bytes = packet->payload.data.size;
                    fseek(file, offset, SEEK_SET);
                    fwrite(packet->payload.data.bytes, sizeof(char),
                           packet->payload.data.size, file);
                    fflush(file);
                } else {
                    printf("Seq already seen: %ld\n", packet->payload.data.seq_num);
                }

                send_ack(sockfd, &src_addr, addrlen, next_seq_num);
                break;
            default:
                fprintf(stderr, "Unknown packet type\n");
        }

        memset(packet, 0, MAX_SEGMENT_SIZE);
    }

    free(packet);
    fclose(file);
    close(sockfd);
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
            arguments->hostname = arg;
            break;
        case 'h':
            arguments->hostname = arg;
            break;
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

    static struct argp_option options[] = {
        {"mode", 'm', "MODE", 0, "Mode", 0},
        {"port", 'p', "PORT", 0, "Mode", 0},
        {"hostname", 'h', "HOSTNAME", 0, "Hostname", 0},
        {0}
    };

    struct argp argp = { options, parse_opt, "", 0, 0, 0, 0};
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    start_receiver(arguments.mode, arguments.port, arguments.hostname);

    return 0;
}
