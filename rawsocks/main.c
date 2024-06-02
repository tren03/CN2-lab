#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/tcp.h> /* TCP Header */
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <linux/ip.h>

#define DEBUG 0
struct sockaddr_in *clientaddr = NULL;
int raw_socket;
int SERVPORT = 50000;
int DESTPORT = 50001;

/* structure to calculate TCP checksum which do not change from the TCP layer and hence
* are used as a part of the TCP checksum */

struct pseudo_iphdr {
    unsigned int source_ip_addr;
    unsigned int dest_ip_addr;
    unsigned char fixed;
    unsigned char protocol;
    unsigned short tcp_len;
};

/* checksum code to calculate TCP checksum
* Code taken from Unix network programming – Richard stevens*/
unsigned short in_cksum(uint16_t *addr, int len) {
    int nleft = len;
    unsigned int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1) {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16); /* add carry */
    answer = (unsigned short)~sum; /* truncate to 16 bits */
    return answer;
}

/* Interrupt_handler – so that CTRL + C can be used to exit the program */
void interrupt_handler(int signum) {
    close(raw_socket);
    free(clientaddr);
    exit(0);
}

#if DEBUG
/* print the IP and TCP headers */
void dumpmsg(unsigned char *recvbuffer, int length) {
    int count_per_length = 40, i = 0;
    for (i = 0; i < count_per_length; i++) {
        printf("%02x ", recvbuffer[i]);
    }
    printf("\n");
}
#endif

int main() {
    socklen_t length, num_of_bytes;
    char buffer[1024] = {0};
    unsigned char recvbuffer[1024] = {0};
    char *string = "Hello client\n";
    struct tcphdr *tcp_hdr = NULL;
    char *string_data = NULL;
    char *recv_string_data = NULL;
    char *csum_buffer = NULL;
    struct pseudo_iphdr csum_hdr;

    signal(SIGINT, interrupt_handler);
    signal(SIGTERM, interrupt_handler);

    if (0 > (raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP))) {
        printf("Unable to create a socket\n");
        exit(0);
    }

    /* Part 2 – create the server connection – fill the structure */
    clientaddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));

    if (clientaddr == NULL) {
        printf("Unable to allocate memory\n");
        goto end;
    }

    clientaddr->sin_family = AF_INET;
    clientaddr->sin_port = htons(DESTPORT);
    clientaddr->sin_addr.s_addr = inet_addr("127.0.0.1");

    memset(buffer, 0, sizeof(buffer));

    /* copy the data after the TCP header */
    string_data = (char *)(buffer + sizeof(struct tcphdr));
    strncpy(string_data, string, strlen(string));

    /* Modify some parameters to send to client in TCP hdr
    * code will perform a syn re-transmit to the receive side */
    tcp_hdr = (struct tcphdr *)buffer;

    tcp_hdr->source = htons(SERVPORT);
    tcp_hdr->dest = htons(DESTPORT);
    tcp_hdr->ack_seq = 0x0; /* seq number */
    tcp_hdr->doff = 5; /* data_offset * 4 is TCP header size */
    tcp_hdr->syn = 1; /* SYN flag */
    tcp_hdr->window = htons(200); /* Window size scaling */

    /* calculate the TCP checksum – based on pseudo IP header + TCP HDR +
    * TCP data. create a buffer and calculate CSUM */

    csum_buffer = (char *)calloc((sizeof(struct pseudo_iphdr) + sizeof(struct tcphdr) + strlen(string_data)), sizeof(char));
    if (csum_buffer == NULL) {
        printf("Unable to allocate csum buffer\n");
        goto end1;
    }

    csum_hdr.source_ip_addr = inet_addr("127.0.0.1");
    csum_hdr.dest_ip_addr = inet_addr("127.0.0.1");
    csum_hdr.fixed = 0;
    csum_hdr.protocol = IPPROTO_TCP; /* TCP protocol */
    csum_hdr.tcp_len = htons(sizeof(struct tcphdr) + strlen(string_data) + 1);

    memcpy(csum_buffer, (char *)&csum_hdr, sizeof(struct pseudo_iphdr));
    memcpy(csum_buffer + sizeof(struct pseudo_iphdr), buffer, (sizeof(struct tcphdr) + strlen(string_data) + 1));

    tcp_hdr->check = (in_cksum((unsigned short *)csum_buffer, (sizeof(struct pseudo_iphdr) + sizeof(struct tcphdr) + strlen(string_data) + 1)));

    printf("checksum is %x\n", tcp_hdr->check);

    /* since we are re-sending the same packet over and over again
    * free the csum buffer here */
    free(csum_buffer);

    while (1) {
        num_of_bytes = sendto(raw_socket, buffer, (sizeof(struct tcphdr) + strlen(string_data) + 1), 0, (struct sockaddr *)clientaddr, sizeof(struct sockaddr_in));
        if (num_of_bytes == -1) {
            printf("unable to send Message\n");
            goto end1;
        }
        sleep(1);
        memset(recvbuffer, 0, sizeof(recvbuffer));
        length = sizeof(struct sockaddr_in);
        num_of_bytes = recvfrom(raw_socket, recvbuffer, sizeof(recvbuffer), 0, (struct sockaddr *)clientaddr, &length);
        if (num_of_bytes == -1) {
            printf("unable to recv Message\n");
            goto end1;
        }
        tcp_hdr = (struct tcphdr *)(recvbuffer + sizeof(struct iphdr));
        recv_string_data = (char *)(recvbuffer + sizeof(struct iphdr) + sizeof(struct tcphdr));

#if DEBUG
        dumpmsg(recvbuffer, (sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen(string_data) + 1));
#endif

        if (SERVPORT == ntohs(tcp_hdr->source)) {
            printf("tcp source is %d, tcp destination is %d, tcp window is %d\n", ntohs(tcp_hdr->source), ntohs(tcp_hdr->dest), ntohs(tcp_hdr->window));
            printf("data is %s\n", recv_string_data);
        }
    }

end1:
    free(clientaddr);
end:
    close(raw_socket);
    return 0;
}
