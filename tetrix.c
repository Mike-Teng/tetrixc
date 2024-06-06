#include "tetrix.h"
#include <stddef.h>  // For NULL
Tetromino TETROMINOES1[7] = {
    { { {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, 1 },
    { { {2, 2}, {2, 2} }, 2 },
    { { {0, 3, 0}, {3, 3, 3}, {0, 0, 0} }, 3 },
    { { {0, 4, 4}, {4, 4, 0}, {0, 0, 0} }, 4 },
    { { {5, 5, 0}, {0, 5, 5}, {0, 0, 0} }, 5 },
    { { {6, 0, 0}, {6, 6, 6}, {0, 0, 0} }, 6 },
    { { {0, 0, 7}, {7, 7, 7}, {0, 0, 0} }, 7 }
};
Tetromino TETROMINOES2[7] = {
    { { {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, 1 },
    { { {2, 2}, {2, 2} }, 2 },
    { { {0, 3, 0}, {3, 3, 3}, {0, 0, 0} }, 3 },
    { { {0, 4, 4}, {4, 4, 0}, {0, 0, 0} }, 4 },
    { { {5, 5, 0}, {0, 5, 5}, {0, 0, 0} }, 5 },
    { { {6, 0, 0}, {6, 6, 6}, {0, 0, 0} }, 6 },
    { { {0, 0, 7}, {7, 7, 7}, {0, 0, 0} }, 7 }
};
Tetromino TETROMINOES_Ori[7] = {
    { { {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, 1 },
    { { {2, 2}, {2, 2} }, 2 },
    { { {0, 3, 0}, {3, 3, 3}, {0, 0, 0} }, 3 },
    { { {0, 4, 4}, {4, 4, 0}, {0, 0, 0} }, 4 },
    { { {5, 5, 0}, {0, 5, 5}, {0, 0, 0} }, 5 },
    { { {6, 0, 0}, {6, 6, 6}, {0, 0, 0} }, 6 },
    { { {0, 0, 7}, {7, 7, 7}, {0, 0, 0} }, 7 }
};

int random_tetromino_list[100];

void init_game(GameBoard *board, int board_num) {
    init_random_tetromino_list();
    board->width = 10;
    board->height = 20;
    board->game_over = 0;
    board->score = 0;
    board->tetromino_next_idx = 1;
    board->tetromino_hold_idx = -1;
    board->current_piece = NULL;
    board->next_piece = get_tetromino_ori(board->tetromino_next_idx);
    board->held_piece = NULL;
    board->can_hold = 1;
    board->current_position.x = 0;
    board->current_position.y = 0;
    for (int i = 0; i < board->height; ++i) {
        for (int j = 0; j < board->width; ++j) {
            board->board[i][j] = 0;
        }
    }
    spawn_new_piece(board, board_num);
}
void init_random_tetromino_list() {
    for (int i = 0; i < 100; ++i) {
        random_tetromino_list[i] = rand() % 7;
    }
}
Tetromino* get_random_tetromino(int idx, int board_num) {
    //return &TETROMINOES[random_tetromino_list[idx%100]];
    if (board_num == 1) {
        return &TETROMINOES1[random_tetromino_list[idx%100]];
    } else if (board_num == 2) {
        return &TETROMINOES2[random_tetromino_list[idx%100]];
    }
}
Tetromino* get_tetromino_ori(int idx) {
    return &TETROMINOES_Ori[random_tetromino_list[idx%100]];
}
void draw_piece(int start_y, int start_x, Tetromino *piece, const char *label) {
    if (!piece) return;
    mvprintw(start_y - 1, start_x, "%s", label);
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            if (piece->shape[y][x]) {
                mvprintw(start_y + y, start_x + x * 2, "[]");
            }
        }
    }
}
void draw_game_area_border(int offset_y, int offset_x, int height, int width) {
    for (int y = 0; y < height; ++y) {
        mvaddch(offset_y + y, offset_x - 1, '|');
        mvaddch(offset_y + y, offset_x + width * 2, '|');
    }
    mvhline(offset_y - 1, offset_x - 1, '-', width * 2 + 2);
    mvhline(offset_y + height, offset_x - 1, '-', width * 2 + 2);
}
void draw_board(GameBoard *board, int offset_y, int offset_x) {
    for (int y = 0; y < board->height; ++y) {
        for (int x = 0; x < board->width; ++x) {
            if (board->board[y][x]) {
                mvprintw(y + offset_y, x * 2 + offset_x, "[]");
            }
        }
    }
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            if (board->current_piece->shape[y][x]) {
                int draw_x = board->current_position.x + x;
                int draw_y = board->current_position.y + y;
                mvprintw(draw_y + offset_y, draw_x * 2 + offset_x, "[]");
            }
        }
    }
}
void spawn_new_piece(GameBoard *board, int board_num) {
    if (board->current_piece == NULL) {
        board->current_piece = get_random_tetromino(0, board_num);
    } else {
        board->current_piece = get_random_tetromino(board->tetromino_next_idx, board_num);
        board->tetromino_next_idx++;
    }
    board->current_position.x = board->width / 2 - 2;
    board->current_position.y = 0;
    board->next_piece = get_tetromino_ori(board->tetromino_next_idx);
    board->can_hold = 1;
    if (!is_valid_position(board, board->current_piece, board->current_position.x, board->current_position.y)) {
        board->game_over = 1;
    }
}
void hold_piece(GameBoard *board, int board_num) {
    if (!board->can_hold) return;
    if (!board->held_piece) {
        board->tetromino_hold_idx = board->tetromino_next_idx - 1;
        board->held_piece = get_tetromino_ori(board->tetromino_next_idx - 1);
        spawn_new_piece(board, board_num);
    } else {
        Tetromino *temp = get_tetromino_ori(board->tetromino_next_idx - 1);
        board->current_piece = get_random_tetromino(board->tetromino_hold_idx, board_num);
        board->tetromino_hold_idx = board->tetromino_next_idx - 1;
        board->held_piece = temp;
        board->current_position.x = board->width / 2 - 2;
        board->current_position.y = 0;
    }
    board->can_hold = 0;
}
int is_valid_position(GameBoard *board, Tetromino *piece, int new_x, int new_y) {
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            if (piece->shape[y][x]) {
                int board_x = new_x + x;
                int board_y = new_y + y;
                if (board_x < 0 || board_x >= board->width || board_y < 0 || board_y >= board->height || board->board[board_y][board_x]) {
                    return 0;
                }
            }
        }
    }
    return 1;
}
void move_piece(GameBoard *board, int direction) {
    int new_x = board->current_position.x + direction;
    if (is_valid_position(board, board->current_piece, new_x, board->current_position.y)) {
        board->current_position.x = new_x;
    }
}
void drop_piece(GameBoard *board, int board_num) {
    int new_y = board->current_position.y + 1;
    if (is_valid_position(board, board->current_piece, board->current_position.x, new_y)) {
        board->current_position.y = new_y;
    } else {
        lock_piece(board);
        spawn_new_piece(board, board_num);
    }
}
void rotate_piece(GameBoard *board) {
    Tetromino new_piece = *board->current_piece;
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            new_piece.shape[x][3 - y] = board->current_piece->shape[y][x];
        }
    }
    if (is_valid_position(board, &new_piece, board->current_position.x, board->current_position.y)) {
        *board->current_piece = new_piece;
    }
}
void update(GameBoard *board, int board_num) {
    drop_piece(board, board_num);
}
void lock_piece(GameBoard *board) {
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            if (board->current_piece->shape[y][x]) {
                int board_x = board->current_position.x + x;
                int board_y = board->current_position.y + y;
                if (board_x >= 0 && board_x < board->width && board_y >= 0 && board_y < board->height) {
                    board->board[board_y][board_x] = board->current_piece->shape[y][x];
                }
            }
        }
    }
    clear_lines(board);
}
void clear_lines(GameBoard *board) {
    int lines_cleared = 0;
    for (int y = 0; y < board->height; ++y) {
        int full_line = 1;
        for (int x = 0; x < board->width; ++x) {
            if (board->board[y][x] == 0) {
                full_line = 0;
                break;
            }
        }
        if (full_line) {
            lines_cleared++;
            for (int row = y; row > 0; --row) {
                for (int col = 0; col < board->width; ++col) {
                    board->board[row][col] = board->board[row - 1][col];
                }
            }
            for (int col = 0; col < board->width; ++col) {
                board->board[0][col] = 0;
            }
        }
    }
    board->score += lines_cleared * 100;
    if(lines_cleared == 4){
        board->score *= 2;
    }
}