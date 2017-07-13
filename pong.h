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

#include "set_ticker.h"

#define SYM_BAL 'o'
#define BLANK ' '
#define WIN_COND 5
#define SLOW 100
#define NORMAL 75
#define FAST 50

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

struct p_ball   ball;
struct p_pc     player;
struct p_npc    nonPlayer;

int diff;
bool score;
bool move_player_paddle;

void setup();
void menu();
void difficulty();
void game();
void update(int);
void update_paddles();
void cleanup();
void add_border();

#endif //PONG_PONG_H