#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>  //socket
#include <sys/socket.h> //socket
#include <string.h>     //memset
#include <stdlib.h>     //sizeof
#include <netinet/in.h> //INADDR_ANY
#include <unistd.h>     //close
#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define MAXSZ 100

int main() {
    int sockfd;                       // to create socket
    struct sockaddr_in serverAddress; // client will connect on this
    int n, n1, n2, ans;
    char op;
    char expression[MAXSZ];

    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // initialize the socket addresses
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddress.sin_port = htons(PORT);

    // client connect to server on port
    if (connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // send to server and receive from server
    while (1) {
        printf("\nEnter the expression (e.g., '3+4', '5*2', '8/2', etc.):\n");
        fgets(expression, MAXSZ, stdin); // Read input as a string

        // Parse the expression
        if (sscanf(expression, "%d %c %d", &n1, &op, &n2) != 3) {
            printf("Invalid expression format!\n");
            continue; // Go to next iteration of the loop
        }

        printf("Sending expression: %d %c %d\n", n1, op, n2);

        // Write the expression to the server
        write(sockfd, &op, sizeof(op));
        write(sockfd, &n1, sizeof(n1));
        write(sockfd, &n2, sizeof(n2));

        // Read the result from the server
        read(sockfd, &ans, sizeof(ans));
        printf("Received result from server: %d\n", ans);
    }

    close(sockfd);
    return 0;
}
