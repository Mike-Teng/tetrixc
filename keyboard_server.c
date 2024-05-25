#include <stdio.h>
#include <unistd.h>
// socket
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // socket
    int port = atoi(argv[1]);
    int socket_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);    

    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if(bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(socket_fd, 3) < 0){
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // Accept the incoming connection
    if((client_socket = accept(socket_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }else{
        printf("Connected\n");
    }

    char buffer[100] = {0};
    while (1) {
        recv(client_socket, buffer, 100, 0);
        // print to stdout
        printf("%s", buffer);
        printf("...\n");
    }

    close(socket_fd);
    return 0;
}