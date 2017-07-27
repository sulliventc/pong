//
// Created by colten on 11/07/17.
//

#ifndef PONG_PONG_H
#define PONG_PONG_H

#include <signal.h>
#include <aio.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "set_ticker.h"

#define SYM_BAL 'o'
#define BLANK ' '
#define WIN_COND 5
#define DIFF_EASY 100
#define DIFF_NORM 75
#define DIFF_HARD 50

struct p_ball {
    int     x_pos, y_pos,
            x_dir, y_dir;
    chtype  symbol;
};

struct p_pc {
    int     x_pos, y_pos,
            score;
};

struct p_npc {
    int     x_pos, y_pos,
            score;
};

struct p_ball   g_ball;
struct p_pc     g_player;
struct p_npc    g_non_player;
struct aiocb    g_kbcbuf;

int g_diff;
int g_player_move;
int g_ai_speed_limiter;
int g_ai_fudge;
bool g_score;

void setup_ncurses();
void cleanup_ncurses();
void main_menu();
void difficulty_menu();
void pause_menu();
void game_loop();
void on_input(int);
void update(int);
void update_paddles();
int  calculate_ai_target();
void add_border();
void setup_aio_buffer();

#endif //PONG_PONG_H