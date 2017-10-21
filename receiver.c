#include "network.h"


int send_ack(int sockfd, const struct sockaddr* dest_addr,
             socklen_t addrlen, long seq_num) {
    struct ack ack;
    struct packet packet;
    memset(&ack, 0, sizeof(struct ack));
    memset(&packet, 0, sizeof(struct packet));

    ack.seq_num = seq_num;

    packet.type = ACK;
    packet.payload.ack = ack;

    printf("Packet Type: %x\nSequence Number: %ld\n",
            packet.type, packet.payload.ack.seq_num);

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
    socklen_t addrlen;

    while (recvfrom(sockfd, packet, MAX_SEGMENT_SIZE, 0,
                    &src_addr, &addrlen)) {
        switch (packet->type) {
            case INIT:
                if (packet->payload.init.mode != mode) {
                    // exit on mode mismatch
                    fprintf(stderr, "Mode mismatch\n");
                    exit(EXIT_FAILURE);
                }
                // create empty file here to write to
                next_seq_num = packet->payload.init.seq_num + 1;
                printf("Sending ack?\n");
                int send_size = send_ack(sockfd, &src_addr, addrlen, next_seq_num);
                if (send_size <=0) {
                    printf("%d", send_size);
                }
                break;
            case DATA:
                // only increase the sequence number if we got what we wanted
                if (packet->payload.data.seq_num == next_seq_num) {
                    printf("Received Data Seq: %ld\n", packet->payload.data.seq_num);
                    next_seq_num = packet->payload.data.seq_num + 1;
                    // Write to disk here
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
}

int main(int argc, char const* argv[])
{
    start_receiver(1, "1337", "127.0.0.1");

    return 0;
}
