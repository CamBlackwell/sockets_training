#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024
#define PORT 8888

int main(){
    struct pollfd clients[MAX_CLIENTS + 1]; //+1 for the server 
    int num_clients = 0;
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;

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
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){ //sets up actual address eg 0.0.0.0:8888
        perror("bind error");
        exit(1);
    }

    if (listen(server_fd, 5) < 0){ //listens for a call from clients (TCP)
        perror("listen failed");
        exit(1);
    }

    printf("server set up and listening on port %d\n", PORT);


    for (int i = 0; i <= MAX_CLIENTS; i++){
        clients[i].fd = -1; //initialise all clients to being empty
    }

    clients[0].fd = server_fd; //server is always 0
    clients[0].events = POLLIN; //meaning watch for incoming connections

    
    while(1){
        int ready = poll(clients, MAX_CLIENTS +1, -1);
        if (ready < 0){
            perror("poll failed");
            break;
        }
        
        if (clients[0].revents & POLLIN){ //checks if server socket has new connection
            
            client_len = sizeof(client_addr);
            client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len); //creates a new socket to handle clients
            if (client_fd < 0){
                perror("accept failed");
                exit(1);
            }

            int slot_found = 0;
            for (int i = 1; i <= MAX_CLIENTS; i++){ // start at 1 since 0 is client
                if (clients[i].fd ==-1){
                    clients[i].fd = client_fd;
                    clients[i].events = POLLIN; //tells to watch for incoming messages
                    num_clients++;
                    slot_found = 1;
                    printf("New client added to slot %d. Total number of clients: %d\n", i, num_clients);
                    break;
                }
            }
            if (!slot_found){
                printf("server full. rejecting client\n");
                close(client_fd);
            }
        }


        for (int i = 1; i <= MAX_CLIENTS; i++){
            if (clients[i].fd == -1) continue;
            if (clients[i].revents & POLLIN){ //bitwise and 
                memset(buffer, 0, BUFFER_SIZE); //calloc
                bytes_read = recv(clients[i].fd, buffer, BUFFER_SIZE -1 , 0); //waits for data from the client and stores it in bytes_read
                
                if (bytes_read <= 0){
                    if (bytes_read == 0){
                        printf("client disconnected, total number of clients: %d\n", num_clients -1);
                    } else {
                        perror("recv error");
                    }
                    close(clients[i].fd);
                    clients[i].fd = -1;
                    num_clients--;
                    continue;
                }            

                printf("received %s", buffer);
                printf("Broadcasting to others... i=%d\n", i);

                for (int j = 1; j <= MAX_CLIENTS; j++){
                    printf("Checking slot %d: fd=%d\n", j, clients[j].fd);
                    
                    if(clients[j].fd != -1 && j != i){
                        printf("Actually sending to slot %d\n", j);
                        
                        if (send(clients[j].fd, buffer, bytes_read, 0) <= 0){
                            perror("send failed");
                            close(clients[j].fd);
                            clients[j].fd = -1;
                            num_clients--;
                        }
                    }
                }
                    
            }
        }

        }

    for (int i = 0; i <= MAX_CLIENTS; i++){
        if (clients[i].fd != -1){
            close(clients[i].fd);
        }
    }
    return 0;

}