#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_MSG_LEN 1024

int main(int argc, char *argv[]) {
    int server_port;
    printf("port: ");
    scanf("%d", &server_port);
    int server_fd, client1_fd, client2_fd;
    struct sockaddr_in server_addr, client1_addr, client2_addr;
    char message[MAX_MSG_LEN];
    int bytes_read;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 2) < 0) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\n", server_port);

    socklen_t client1_addr_len = sizeof(client1_addr);
    client1_fd = accept(server_fd, (struct sockaddr *) &client1_addr, &client1_addr_len);
    if (client1_fd < 0) {
        perror("Error accepting client connection");
        exit(EXIT_FAILURE);
    }

    printf("Client 1 connected\n");

    socklen_t client2_addr_len = sizeof(client2_addr);
    client2_fd = accept(server_fd, (struct sockaddr *) &client2_addr, &client2_addr_len);
    if (client2_fd < 0) {
        perror("Error accepting client connection");
        exit(EXIT_FAILURE);
    }

    printf("Client 2 connected\n");


    while (1) {
        memset(message, 0, MAX_MSG_LEN);
        bytes_read = read(client1_fd, message, MAX_MSG_LEN);
        if (bytes_read < 0) {
            perror("Error reading message from client");
            break;
        } else if (bytes_read == 0) {
            printf("Client 1 disconnected\n");
            break;
        }

        printf("Received message: %s", message);

        if (send(client2_fd, message, strlen(message), 0) < 0) {
            perror("Error sending message to client");
            break;
        }

        if (strcmp(message, "The End\n") == 0) {
            printf("Shutting down server and clients\n");
            break;
        }
    }

    close(client1_fd);
    close(client2_fd);
    close(server_fd);

    return 0;
}