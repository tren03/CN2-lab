#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 5001
#define SERVER_IP "127.0.0.1"
#define MAXSZ 100

int main() {
    int sockfd;
    struct sockaddr_in serverAddress;
    int n;
    char msg1[MAXSZ];
    char msg2[MAXSZ];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddress.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("\nEnter message to send to server: ");
        fgets(msg1, MAXSZ, stdin);

        if (msg1[0] == '#') {
            break;
        }

        n = strlen(msg1);
        send(sockfd, msg1, n, 0);

        n = recv(sockfd, msg2, MAXSZ, 0);
        msg2[n] = '\0';
        printf("Received message from server: %s\n", msg2);
    }

    close(sockfd);
    return 0;
}
