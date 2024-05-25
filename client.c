#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
// socket
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


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

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    // socket
    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    struct sockaddr_in server_addr;

    // build socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("socket");
        return -1;
    }

    // set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    // convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        return -1;
    }

    // connect to server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return -1;
    }else{
        // printf("Connected\n");
    }
    
    char ch;
    printf("請按上下左右鍵來輸出方向。\n");
    printf("按 'q' 退出程式。\n");

    enableRawMode();

    while (1) {
        if (kbhit()) {
            ch = getchar();
            if (ch == 'q') {
                send(client_fd, "6", sizeof("q"), 0);
                break;
            }

            if (ch == 27) { // 特殊鍵的開始碼
                getchar(); // 跳過 '['
                ch = getchar();
                switch (ch) {
                    case 'A': // 上箭頭鍵
                        printf("UP\n");
                        send(client_fd, "4", sizeof("UP"), 0);
                        // sleep(1);
                        break;
                    case 'B': // 下箭頭鍵
                        printf("DOWN\n");
                        send(client_fd, "3", sizeof("DOWN"), 0);
                        // sleep(1);
                        break;
                    case 'D': // 左箭頭鍵
                        printf("LEFT\n");
                        send(client_fd, "1", sizeof("LEFT"), 0);
                        // sleep(1);
                        break;
                    case 'C': // 右箭頭鍵
                        printf("RIGHT\n");
                        send(client_fd, "2", sizeof("RIGHT"), 0);
                        // sleep(1);
                        break;
                    default:
                        break;
                }
            }
            else if (ch == ' '){
                printf("SPACE\n");
                send(client_fd, "5", sizeof("SPACE"), 0);
            }
        }
    }

    disableRawMode();
    close(client_fd);
    return 0;
}
