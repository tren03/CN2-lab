#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>  //socket
#include <sys/socket.h> //socket
#include <string.h>     //memset
#include <stdlib.h>     //sizeof
#include <netinet/in.h> //INADDR_ANY
#include <unistd.h>     //close
#define PORT 8080
#define MAXSZ 100
int main()
{
    int sockfd;                       // to create socket
    int newsockfd;                    // to accept connection
    struct sockaddr_in serverAddress; // server receive on this address
    struct sockaddr_in clientAddress; // server sends to client on this address
    int n;
    char msg[MAXSZ];
    int clientAddressLength;
    int pid;
    int n1, n2, ans;
    char op;
    // create socket

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    // initialize the socket addresses
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(PORT);
    // bind the socket with the server address and port
    if (bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    // listen for connection from client
    if (listen(sockfd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        // parent process waiting to accept a new connection
        printf("\n*****server waiting for new client connection:*****\n");
        clientAddressLength = sizeof(clientAddress);
        newsockfd = accept(sockfd, (struct sockaddr *)&clientAddress, (socklen_t*)&clientAddressLength);
        if (newsockfd < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        printf("connected to client: %s\n", inet_ntoa(clientAddress.sin_addr));
        // child process is created for serving each new clients
        pid = fork();
        if (pid == 0) // child process rec and send
        {
            close(sockfd); // Close the listening socket in child process
            // receive from client
            while (1)
            {
                read(newsockfd, &op, sizeof(op));
                read(newsockfd, &n1, sizeof(n1));
                read(newsockfd, &n2, sizeof(n2));
                printf("%d\t%d\t%c\n", n1, n2, op);
                switch (op)
                {
                    case '+':
                        ans = n1 + n2;
                        break;
                    case '-':
                        ans = n1 - n2;
                        break;
                    case '*':
                        ans = n1 * n2;
                        break;
                    case '/':
                        ans = n1 / n2;
                        break;
                    default:
                        ans = 0; // Handle invalid operation
                        break;
                }
                write(newsockfd, &ans, sizeof(ans));
                printf("Received and sent: %d\n", ans);
            }
            exit(0);
        }
        else
        {
            close(newsockfd); // Close the connected socket in parent process
        }
    } // close exterior while

    return 0;
}
