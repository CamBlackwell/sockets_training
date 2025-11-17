#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE = 1024;
#define PORT = 8888;

int main(){
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    struct sock_len client_len;

    char buffer[BUFFER_SIZE];
    int bytes_read;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);//ceates a socket for server 
    if (server_fd < 0){
        perror("socket error");
        exit(1);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){ //allows for reuse of recent addresses 
        perror("SO_REUSEADDR error");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr)); //sets up standards for server ipv4
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDER_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, &server_addr, sizeof(server_addr)) < 0){ //sets up actual address eg 0.0.0.0:8888
        perror("bind error");
        exit(1);
    }

    if (listen(server_fd, 5) < 0){ //listens for a call from clients (TCP)
        perror("listen failed");
        exit(1);
    }

    printf("server set up and listening on port %d/n", PORT);

    client_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr); //creates a new socket to handle clients
    if (client_fd < 0){
        perror("accept failed");
        exit(1);
    }

    prinf("Client connected from%s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    
    while(1){
        memset(buffer, 0, BUFFER_SIZE); //calloc
        bytes_read = recv(client_fd, buffer, BUFFER_SIZE -1 , 0); //waits for data from the client and stores it in bytes_read
        
        if (bytes_read < 0){
            if (bytes_read == 0){
                printf("client disconnected\n")
            } else {
                perror("recv error");
            }
            break;
        }            

        printf("recieved %s", buffer);
        if (send(client_fd, bytes_read, 0) < 0){ //sends message back to sender
            perror("send failed");
            break;
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;

}