#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include "tetrix.h"

int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    timeout(500);
    start_color();

    for (int i = 1; i <= 7; ++i) {
        init_pair(i, i, -1);
    }

    srand(time(0)); // Seed the random number generator

    GameBoard board;
    init_game(&board);

    while (1) {
        clear();
        if (board.game_over) {
            mvprintw(0, 10, "Game Over! Press 'q' to quit.");
            mvprintw(1, 10, "Final Score: %d", board.score);
            if (getch() == 'q') {
                break;
            }
            continue;
        }

        // Display score
        mvprintw(0, 1, "Score: %d", board.score);
        
        // Draw game area border
        draw_game_area_border(2, 10, board.height, board.width);
        
        // Display next and held pieces
        draw_piece(1, 32, board.next_piece, "Next:");
        draw_piece(10, 32, board.held_piece, "Hold:");

        // Display the board with the current piece
        draw_board(&board);

        // Handle input
        int ch = getch();
        if (ch == KEY_LEFT) {
            move_piece(&board, -1);
        } else if (ch == KEY_RIGHT) {
            move_piece(&board, 1);
        } else if (ch == KEY_DOWN) {
            drop_piece(&board);
        } else if (ch == KEY_UP) {
            rotate_piece(&board);
        } else if (ch == ' ') {  // Space bar to hold
            hold_piece(&board);
        }

        update(&board);
        refresh();
    }

    endwin();
    return 0;
}
