//
// Created by colten on 11/07/17.
//

#ifndef PONG_PONG_H
#define PONG_PONG_H

#include <signal.h>
#include <aio.h>
#include <ncurses.h>
#include <unistd.h>

#include "set_ticker.h"

#define SYM_BAL 'o'
#define SYM_PAD '|'
#define BLANK ' '
#define WIN_COND 10
#define SLOW 600
#define FAST 500

struct p_ball {
    int     x_pos, y_pos,
            x_dir, y_dir;
    char    symbol;
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

void setup();
void game_loop();
void cleanup();

#endif //PONG_PONG_H