#include "pong.h"

int main(int argc, char *argv[]) {
    setup();
    game_loop();
    cleanup();

    return 0;
}

void setup() {
    initscr();
    wresize(stdscr, 25, 81);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_BLUE);
    attron(COLOR_PAIR(1));
    noecho();
    crmode();
    curs_set(0);
}

void game_loop() {
    getch();
    mvaddstr(2, 2, )
}

void cleanup() {
    attroff(COLOR_PAIR(1));
    set_ticker(0);
    endwin();
}