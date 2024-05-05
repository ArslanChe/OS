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

    printf("Connected to server at %s:%d\n", server_ip, server_port);

    while (1) {
        fgets(message, MAX_MSG_LEN, stdin);
        if (send(client_fd, message, strlen(message), 0) < 0) {
            perror("Error sending message to server");
            break;
        }

        if (strcmp(message, "The End\n") == 0) {
            printf("Shutting down client\n");
            break;
        }
    }

    close(client_fd);

    return 0;
}