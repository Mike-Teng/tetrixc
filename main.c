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

#include <pthread.h>

// keyboard
#include <termios.h>
#include <fcntl.h>
#include <sys/select.h>

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
char board1_char[3000];
char board2_char[3000];
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;

void enableRawMode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void disableRawMode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;
    fd_set set;
    struct timeval tv;

    // Save old terminal attributes
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Set non-blocking mode
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    int result = select(STDIN_FILENO + 1, &set, NULL, NULL, &tv);
    if (result > 0) {
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        if (ch != EOF) {
            ungetc(ch, stdin);
            return 1;
        }
    }

    // Restore old terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    return 0;
}

// timer handler
void timer_handler(int signum) {
    // send game board to client
    // printf("Send game board to client\n");   

    // pthread_mutex_lock(&mutex1);
    // serialize_game_board(&board1, board1_char);
    // pthread_mutex_unlock(&mutex1);

    // send(client1_socket, board1_char, sizeof(board1_char), 0);
    // serialize_game_board(&board2, board2_char);
    // send(client2_socket, board2_char, sizeof(board2_char), 0);
    if(send_count < 5){
        send_count++;
    }
    clear();

    draw_board(&board1, 2, 10);
    draw_board(&board2, 2, 50);
    mvprintw(0, 1, "Player 1 Score: %d", board1.score);
    mvprintw(0, 40, "Player 2 Score: %d", board2.score);
    draw_game_area_border(2, 10, board1.height, board1.width);
    draw_game_area_border(2, 50, board2.height, board2.width);
    draw_piece(1, 32, board1.next_piece, "Next:");
    draw_piece(10, 32, board1.held_piece, "Hold:");
    draw_piece(1, 72, board2.next_piece, "Next:");
    draw_piece(10, 72, board2.held_piece, "Hold:");
    refresh();
    // printf("send time: %f\n", send_count*0.1);
}

// zombie process handler
void sigchld_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void client_handler_kb1() {
    int keys[256] = {0};

    enableRawMode();
    while (1) {
        if (kbhit()) {
            char ch = getchar();
            keys[(int)ch] = 1;
        }

        pthread_mutex_lock(&mutex1);

        if (keys['w']) {
            strcpy(shm1, "4");
        } else if (keys['a']) {
            strcpy(shm1, "1");
        } else if (keys['s']) {
            strcpy(shm1, "3");
        } else if (keys['d']) {
            strcpy(shm1, "2");
        } else if (keys['z']) {
            strcpy(shm1, "5");
        } else if (keys['/']) {
            strcpy(shm2, "5");
        } else if (keys[27]) { // Handle arrow keys
            char seq[3];
            seq[0] = getchar();
            seq[1] = getchar();
            if (seq[0] == '[') {
                if (seq[1] == 'A') {
                    strcpy(shm2, "4");
                } else if (seq[1] == 'B') {
                    strcpy(shm2, "3");
                } else if (seq[1] == 'C') {
                    strcpy(shm2, "2");
                } else if (seq[1] == 'D') {
                    strcpy(shm2, "1");
                }
            }
        }

        pthread_mutex_unlock(&mutex1);

        // Reset key state after handling
        for (int i = 0; i < 256; ++i) {
            keys[i] = 0;
        }
    }
    disableRawMode();
    exit(0);
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
    exit(0);
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

    // There are two modes: battle on the same computer or battle on different computers
    int mode;
    printf("Choose the mode: 1 for play on local, 2 for play online\n");
    scanf("%d", &mode);

    int port = atoi(argv[2]);
    int socket_fd;
    char *server_ip = argv[1];
    
    if(mode == 2){
        printf("Online Game Mode Selected\n");
        // socket
        struct sockaddr_in address;
        int addrlen = sizeof(address);

        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("Socket Connection failed");
            exit(EXIT_FAILURE);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(server_ip);
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
            return 0;
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
            return 0;
        }
    }else if(mode == 1){
        printf("Local Game Mode Selected\n");
        if(fork() == 0){
            client_handler_kb1();
            return 0;
        }
    }

    // main game loop in parent process
    {   
        initscr();
        cbreak();
        noecho();
        curs_set(FALSE);
        start_color();

        init_game(&board1,1);
        init_game(&board2,2);

        // start timer
        setitimer(ITIMER_VIRTUAL, &timer, NULL);

        while (1) {
            if (board1.game_over && board2.game_over) {
                clear();
                mvprintw(0, 10, "Game Over!");
                mvprintw(1, 10, "Final Scores:");
                mvprintw(2, 10, "Player 1: %d", board1.score);
                mvprintw(3, 10, "Player 2: %d", board2.score);
                // printf("Game over\n");
                refresh();
                sleep(1);
                continue;
            }
            // Get command from shared memory
            int cmd1 = 0, cmd2 = 0;
            
            pthread_mutex_lock(&mutex3);
            if (strcmp(shm1, "8") != 0) {
                if(!board1.game_over)
                    cmd1 = atoi(shm1);
                strcpy(shm1, "8");
            }

            if (strcmp(shm2, "8") != 0) {
                if(!board2.game_over)
                    cmd2 = atoi(shm2);
                strcpy(shm2, "8");
            }
            pthread_mutex_unlock(&mutex3);
            // if(cmd1!=0 || cmd2 != 0)
            //     printf("cmd1: %d, cmd2: %d\n", cmd1, cmd2);

            // Handle the command
            if (cmd1 == LEFT) {
                move_piece(&board1, -1);
            } else if (cmd1 == RIGHT) {
                move_piece(&board1, 1);
            } else if (cmd1 == DOWN) {
                drop_piece(&board1,1);
            } else if (cmd1 == UP) {
                rotate_piece(&board1);
            } else if (cmd1 == SPACE) {  // Space bar to hold
                hold_piece(&board1,1);
            }
            
            if (cmd2 == LEFT) {
                move_piece(&board2, -1);
            } else if (cmd2 == RIGHT) {
                move_piece(&board2, 1);
            } else if (cmd2 == DOWN) {
                drop_piece(&board2,2);
            } else if (cmd2 == UP) {
                rotate_piece(&board2);
            } else if (cmd2 == SPACE) {  // Space bar to hold
                hold_piece(&board2,2);
            }

            
            

            if (send_count >= 5) {
                if(!board1.game_over)
                    update(&board1,1);
                if(!board2.game_over)
                    update(&board2,2);
                send_count = 0;
            }
            // refresh();
        }
    }
    // detach shared memory
    shmdt(shm1);
    shmdt(shm2);
    // destroy shared memory
    shmctl(shmid1, IPC_RMID, NULL);
    shmctl(shmid2, IPC_RMID, NULL);
    if(mode == 2){
        close(socket_fd);
        close(client1_socket);
        close(client2_socket);
    }
    return 0;
}
