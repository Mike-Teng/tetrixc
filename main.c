#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include "tetrix.h"
// shared memory
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define LEFT 1
#define RIGHT 2
#define DOWN 3
#define UP 4
#define SPACE 5
#define QUIT 6

#define SHM_SIZE 10

int main(int argc, char **argv) {

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

    char* data = "8";
    strcpy(shm, data);
    
    

    srand(time(0)); // Seed the random number generator

    int port = atoi(argv[1]);
    int socket_fd, client_socket;
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

    if ((client_socket = accept(socket_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0){
        perror("Socket accept failed");
        exit(EXIT_FAILURE);
    }
    // else{   
    //     // printf("Client connected\n");
    // }

    int pid = fork();
    if (pid == 0) {
        close(socket_fd);
        while (1) {
            // receive data from socket
            char input[10] = {0};
            int valread = recv(client_socket, input, 10, 0);
            if (valread == 0) {
                break;
            }
            // write to shared memory
            strcpy(shm, input);
        }
        close(client_socket);
    }

    if (pid >0) {

    initscr();
    cbreak();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    // timeout(500);
    start_color();

    for (int i = 1; i <= 7; ++i) {
        init_pair(i, i, -1);
    }

    GameBoard board;
    init_game(&board);

    while (1) {
        clear();
        int get_cmd_flag = 0;
        int cmd = 0;
        if (strcmp(shm, "8") != 0) {
            cmd = atoi(shm);
            get_cmd_flag = 1;
            strcpy(shm, "8");
        }

        if (board.game_over) {
            mvprintw(0, 10, "Game Over! Press 'q' to quit.");
            mvprintw(1, 10, "Final Score: %d", board.score);
            if (cmd == QUIT) {
                break;
            }
            sleep(1);
            refresh();
            continue;
        }

        // mvprintw(20, 32, "Key pressed: %d", cmd);
        // Display score
        mvprintw(0, 1, "Score: %d", board.score);
        
        // Draw game area border
        draw_game_area_border(2, 10, board.height, board.width);
        
        // Display next and held pieces
        draw_piece(1, 32, board.next_piece, "Next:");
        draw_piece(10, 32, board.held_piece, "Hold:");

        // Handle input
        if (cmd == LEFT) {
            move_piece(&board, -1);
        } else if (cmd == RIGHT) {
            move_piece(&board, 1);
        } else if (cmd == DOWN) {
            drop_piece(&board);
        } else if (cmd == UP) {
            rotate_piece(&board);
        } else if (cmd == SPACE) {  // Space bar to hold
            hold_piece(&board);
        }
        // Display the board with the current piece
        draw_board(&board);


        update(&board);
        refresh();
        if (get_cmd_flag == 0) 
            usleep(300*1000);
    }
    }
    // detach shared memory
    shmdt(shm);
    // destroy shared memory
    shmctl(shmid, IPC_RMID, NULL);
    endwin();
    close(socket_fd);
    return 0;
}
