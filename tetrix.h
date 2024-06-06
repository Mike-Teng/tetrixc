#ifndef TETRIX_H
#define TETRIX_H
#include <ncurses.h>
#include <stdlib.h>
#include <stddef.h>  // For NULL
#define MAX_HEIGHT 20
#define MAX_WIDTH 10
typedef struct {
    char shape[4][4];
    int color;
} Tetromino;
typedef struct {
    int width;
    int height;
    int board[MAX_HEIGHT][MAX_WIDTH];
    int tetromino_next_idx;
    int tetromino_hold_idx;
    Tetromino *current_piece;
    Tetromino *next_piece;
    Tetromino *held_piece;
    struct {
        int x;
        int y;
    } current_position;
    int game_over;
    int score;
    int can_hold;
} GameBoard;
void init_game(GameBoard *board, int board_num);
void init_random_tetromino_list();
Tetromino* get_random_tetromino(int idx, int board_num);
Tetromino* get_tetromino_ori(int idx);
void draw_piece(int start_y, int start_x, Tetromino *piece, const char *label);
void draw_game_area_border(int offset_y, int offset_x, int height, int width);
void draw_board(GameBoard *board, int offset_y, int offset_x);
void spawn_new_piece(GameBoard *board, int board_num);
void hold_piece(GameBoard *board, int board_num);
int is_valid_position(GameBoard *board, Tetromino *piece, int new_x, int new_y);
void move_piece(GameBoard *board, int direction);
void drop_piece(GameBoard *board, int board_num);
void rotate_piece(GameBoard *board);
void update(GameBoard *board, int board_num);
void lock_piece(GameBoard *board);
void clear_lines(GameBoard *board);
#endif