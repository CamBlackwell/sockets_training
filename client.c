#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main(){
    int client_fd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    int bytes_read;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);//creates a socket for server 
    if (client_fd < 0){
        perror("socket error");
        exit(1);
    }


    memset(&server_addr, 0, sizeof(server_addr)); //sets up a server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "192.168.x.x", &server_addr.sin_addr) <= 0){ //turns to binary 
        perror("invalid address");
        exit(1);
    }

    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("failed to connect");
        exit(1);
    }

    printf("connected to server. type messages CTRL D to quit :)\n");

    while(fgets(buffer, BUFFER_SIZE, stdin) != NULL ){ //loops and puts stdin into buffer
        if (send(client_fd, buffer, strlen(buffer), 0) < 0){
            perror("send error");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        bytes_read = recv(client_fd, buffer, BUFFER_SIZE-1, 0);
        if (bytes_read <= 0){
            printf("server disconnected");
            break;
        }

        printf("server echoes: %s", buffer);

    }
    close(client_fd);
    return 0;
}