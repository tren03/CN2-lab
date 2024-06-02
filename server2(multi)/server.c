#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <string.h> 

#define PORT 8000 
#define MAXSZ 100 

int main() { 
    int sockfd, newsockfd; 
    struct sockaddr_in servaddr, clientaddr; 
    int n; 
    char msg[MAXSZ]; 
    socklen_t cli_len; 
    int pid; 

    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 5) < 0) {
        perror("Listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1) { 
        printf("\nServer waiting for new client connection...\n"); 
        cli_len = sizeof(clientaddr); 
        newsockfd = accept(sockfd, (struct sockaddr*)&clientaddr, &cli_len); 
        if (newsockfd < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Connected to client: %s\n", inet_ntoa(clientaddr.sin_addr)); 
        pid = fork(); 

        if (pid < 0) {
            perror("Fork failed");
            close(newsockfd);
            continue;
        }

        if (pid == 0) { // Child process
            close(sockfd); // Close the listening socket in the child process
            while (1) { 
                n = recv(newsockfd, msg, MAXSZ, 0); 
                if (n == 0) { 
                    printf("Client disconnected: %s\n", inet_ntoa(clientaddr.sin_addr));
                    close(newsockfd); 
                    break; 
                } 
                msg[n] = 0; 
                send(newsockfd, msg, n, 0); 
                printf("Received and sent: %s\n", msg); 
            } 
            exit(0); 
        } else { 
            close(newsockfd); // Close the connected socket in the parent process
        } 
    } 

    close(sockfd);
    return 0;
}
