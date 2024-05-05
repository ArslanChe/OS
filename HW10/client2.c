#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_MSG_LEN 1024

int main(int argc, char *argv[]) {
    int server_port;
    char server_ip[100];
    printf("port: ");
    scanf("%d", &server_port);

    printf("ip: ");
    scanf("%s", server_ip);

    int client_fd;
    struct sockaddr_in server_addr;
    char message[MAX_MSG_LEN];
    int bytes_read;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

    if (connect(client_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int num_bytes = read(client_fd, message, sizeof(message));
        if (num_bytes < 0) {
            perror("Error reading from socket");
            exit(EXIT_FAILURE);
        }
        message[num_bytes] = '\0';
        printf("Received message from server: %s\n", message);
        if (strcmp(message, "The End\n") == 0) {
            printf("Shutting down client\n");
            break;
        }
    }

    close(client_fd);

    return 0;
}

