#include <stdio.h>
#include <unistd.h>
// socket
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
// shared memory
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define SHM_SIZE 10

int main(int argc, char *argv[]) {
    // shared memory
    key_t key = 1917;
    char *shm;
    int shmid;

    // create shared memory
    if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    // put data into shared memory
    char *data = "8";
    strcpy(shm, data);
    
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

    int pid = fork();
    if (pid == 0) {
        close(socket_fd);
        while (1) {
            // receive data from socket
            char buffer[100] = {0};
            int valread = recv(client_socket, buffer, 100, 0);
            if (valread == 0) {
                break;
            }
            // write to shared memory
            strcpy(shm, buffer);
        }
        close(client_socket);
    }

    printf("I'm in parent process\n");
    // get data from shared memory
    char buffer[100] = {0};
    while (1) {
        strcpy(buffer, shm);
        // print to stdout
        printf("%s", buffer);
        printf("...\n");
    }

    // char buffer[100] = {0};
    // while (1) {
    //     recv(client_socket, buffer, 100, 0);
    //     // print to stdout
    //     printf("%s", buffer);
    //     printf("...\n");
    // }

    // detach shared memory
    shmdt(shm);
    // destroy shared memory
    shmctl(shmid, IPC_RMID, NULL);

    close(socket_fd);
    return 0;
}