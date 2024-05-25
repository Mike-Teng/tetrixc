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

void init_game(GameBoard *board);
Tetromino* get_random_tetromino();
void draw_piece(int start_y, int start_x, Tetromino *piece, const char *label);
void draw_game_area_border(int offset_y, int offset_x, int height, int width);
void draw_board(GameBoard *board);
void spawn_new_piece(GameBoard *board);
void hold_piece(GameBoard *board);
int is_valid_position(GameBoard *board, Tetromino *piece, int new_x, int new_y);
void move_piece(GameBoard *board, int direction);
void drop_piece(GameBoard *board);
void rotate_piece(GameBoard *board);
void update(GameBoard *board);
void lock_piece(GameBoard *board);
void clear_lines(GameBoard *board);

#endif
