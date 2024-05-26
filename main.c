#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include "tetrix.h"
// socket
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
// shared memory
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
// timer
#include <stdio.h>
#include <sys/time.h>
#include<features.h>
#include <signal.h>
#include <sys/wait.h>

// Define key codes
#define LEFT 1
#define RIGHT 2
#define DOWN 3
#define UP 4
#define SPACE 5
#define QUIT 6

// Shared memory size
#define SHM_SIZE 5

// game board
GameBoard board1, board2;
int send_count = 0;
int client1_socket, client2_socket;
char *shm1, *shm2;

// timer handler
void timer_handler(int signum) {
    // send game board to client
    printf("Send game board to client\n");
    if(send_count < 5){
        send_count++;
    }
    printf("send time: %f\n", send_count*0.1);
}

// zombie process handler
void sigchld_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void client_handler(int client_socket, int player) {
    char buffer[5] = {0};
    int valread;
    while (1) {
        valread = recv(client_socket, buffer, 5, 0);
        if (valread == 0) {
            break;
        }
        if(player == 1){
            strcpy(shm1, buffer);
        }else{
            strcpy(shm2, buffer);
        }
    }
    close(client_socket);
}

int main(int argc, char **argv) {

    // set timer
    struct sigaction sa;
    struct itimerval timer;

    // Install timer_handler as the signal handler for SIGVTALRM.
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timer_handler;
    sigaction(SIGVTALRM, &sa, NULL);

    // Configure the timer to expire after 100 msec...
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 100000;

    // ... and every 100 msec after that.
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 100000;


    // shared memory
    key_t key1 = 1917, key2 = 2017;
    
    int shmid1, shmid2;

    // create shared memory
    if ((shmid1 = shmget(key1, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget 1");
        exit(1);
    }

    if ((shmid2 = shmget(key2, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget 2");
        exit(1);
    }

    // attach shared memory
    if ((shm1 = shmat(shmid1, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    if ((shm2 = shmat(shmid2, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    // initialize shared memory
    char* data = "8";
    strcpy(shm1, data);
    strcpy(shm2, data);
    
    // Seed the random number generator
    srand(time(0)); 

    // socket
    int port = atoi(argv[1]);
    int socket_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket Connection failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(socket_fd, 3) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // create a child process for client1
    signal(SIGCHLD, sigchld_handler);

    // accept client1 connection
    printf("Waiting for client1 connection...\n");
    if ((client1_socket = accept(socket_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0){
        perror("Socket accept failed");
        exit(EXIT_FAILURE);
    }
    else{   
        printf("client1_socket: %d\n", client1_socket);
        printf("Client1 connected\n");
    }

    if (fork() == 0) {
        close(socket_fd);
        client_handler(client1_socket, 1);
    }
    
    // accept client2 connection
    printf("Waiting for client2 connection...\n");
    client2_socket = accept(socket_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if ( client2_socket < 0){
        perror("Socket accept failed");
        exit(EXIT_FAILURE);
    }
    else if (client2_socket > 0){
        printf("client2_socket: %d\n", client2_socket);
        printf("Client2 connected\n");

    }

    if (fork() == 0) {
        close(socket_fd);
        client_handler(client2_socket, 2);
    }

    // main game loop in parent process
    {
        init_game(&board1);
        init_game(&board2);

        // start timer
        setitimer(ITIMER_VIRTUAL, &timer, NULL);

        while (1) {
            // Get command from shared memory
            int cmd1 = 0, cmd2 = 0;
            if (strcmp(shm1, "8") != 0) {
                cmd1 = atoi(shm1);
                strcpy(shm1, "8");
            }

            if (strcmp(shm2, "8") != 0) {
                cmd2 = atoi(shm2);
                strcpy(shm2, "8");
            }
            
            if(cmd1!=0 || cmd2 != 0)
                printf("cmd1: %d, cmd2: %d\n", cmd1, cmd2);

            if (board1.game_over || board2.game_over) {
                printf("Game over\n");
                break;
            }

            // Handle the command
            if (cmd1 == LEFT) {
                move_piece(&board1, -1);
            } else if (cmd1 == RIGHT) {
                move_piece(&board1, 1);
            } else if (cmd1 == DOWN) {
                drop_piece(&board1);
            } else if (cmd1 == UP) {
                rotate_piece(&board1);
            } else if (cmd1 == SPACE) {  // Space bar to hold
                hold_piece(&board1);
            }
            
            if (cmd2 == LEFT) {
                move_piece(&board2, -1);
            } else if (cmd2 == RIGHT) {
                move_piece(&board2, 1);
            } else if (cmd2 == DOWN) {
                drop_piece(&board2);
            } else if (cmd2 == UP) {
                rotate_piece(&board2);
            } else if (cmd2 == SPACE) {  // Space bar to hold
                hold_piece(&board2);
            }

            if (send_count >= 5) {
                update(&board1);
                update(&board2);
                send_count = 0;
            }
        }
    }
    // detach shared memory
    shmdt(shm1);
    shmdt(shm2);
    // destroy shared memory
    shmctl(shmid1, IPC_RMID, NULL);
    shmctl(shmid2, IPC_RMID, NULL);
    close(socket_fd);
    close(client1_socket);
    close(client2_socket);  
    return 0;
}
