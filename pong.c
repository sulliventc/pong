#include "pong.h"

int main(int argc, char *argv[]) {
    setup();
    menu();
    cleanup();

    return 0;
}

void setup() {
    initscr();
    if (COLS <= 40 || LINES <= 12) {
        mvprintw(0,0,"Play area too small.");
        mvprintw(1,0,"Recommended size: 80 x 24");
        refresh();
        getch();
        endwin();
        exit(EXIT_SUCCESS);
    }
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    noecho();
    crmode();
    curs_set(0);
}

void menu() {
    clear();
    add_border();
    mvprintw(4,  (COLS/2)-22, "$$$$$$$\\   $$$$$$\\  $$\\   $$\\  $$$$$$\\  $$\\ ");
    mvprintw(5,  (COLS/2)-22, "$$  __$$\\ $$  __$$\\ $$$\\  $$ |$$  __$$\\ $$ |");
    mvprintw(6,  (COLS/2)-22, "$$ |  $$ |$$ /  $$ |$$$$\\ $$ |$$ /  \\__|$$ |");
    mvprintw(7,  (COLS/2)-22, "$$$$$$$  |$$ |  $$ |$$ $$\\$$ |$$ |$$$$\\ $$ |");
    mvprintw(8,  (COLS/2)-22, "$$  ____/ $$ |  $$ |$$ \\$$$$ |$$ |\\_$$ |\\__|");
    mvprintw(9,  (COLS/2)-22, "$$ |      $$ |  $$ |$$ |\\$$$ |$$ |  $$ |    ");
    mvprintw(10, (COLS/2)-22, "$$ |       $$$$$$  |$$ | \\$$ |\\$$$$$$  |$$\\ ");
    mvprintw(11, (COLS/2)-22, "\\__|       \\______/ \\__|  \\__| \\______/ \\__|");
    mvprintw(13, (COLS/2)-22, "Control with UP and DOWN arrows.");
    mvprintw(14, (COLS/2)-22, "Press any key to start!");

    // TODO: Restrict play area properly
    if (COLS > 80 || LINES > 24) {
        mvprintw(LINES-4, (COLS/2)-28, "NOTE: This game intended for smaller play area.");
        mvprintw(LINES-3, (COLS/2)-28, "      Recommended dimensions: 80 x 24");
    }

    refresh();
    getch();
    difficulty();
}

void difficulty() {
    clear();
    add_border();

    mvprintw(4, (COLS/2)-22, "Select Ball Speed:");
    mvprintw(6, (COLS/2)-22, "1. Easy - Slow Ball Speed");
    mvprintw(8, (COLS/2)-22, "2. Medium - Normal Ball Speed");
    mvprintw(10, (COLS/2)-22, "3. Hard - Fast Ball Speed");

    refresh();
    diff = getch() - '0';
    (diff > 0 && diff < 4 ? game : difficulty)();
}

void game() {
    clear();
    add_border();
    mvvline(0, COLS / 2, ACS_VLINE, LINES);
    mvaddch(0, COLS / 2, ACS_TTEE);
    mvaddch(LINES-1, COLS / 2, ACS_BTEE);

    setup_aio_buffer();
    aio_read(&kbcbuf);
    signal(SIGIO, on_input);
    signal(SIGALRM, update);
    set_ticker(NORMAL);

    player.score = nonPlayer.score = 0;
    ball.x_dir = ball.y_dir = 1;
    ball.symbol = SYM_BAL;

    while (player.score != WIN_COND && nonPlayer.score != WIN_COND) {
        score = false;

        ball.x_pos = COLS/2;
        ball.y_pos = LINES/2;
        ball.x_dir *= -1;
        ball.y_dir *= -1;

        nonPlayer.x_pos = 2;
        nonPlayer.y_pos = (LINES/2) - 2;
        player.x_pos = COLS-3;
        player.y_pos = (LINES/2) - 2;

        while (!score) {
            refresh();
        }
    }
}

void on_input(int signum) {
    int c;
    char *cp = (char *) kbcbuf.aio_buf;
    mvaddch(2, 2, 'x');

    if (aio_error(&kbcbuf) != 0) {
        perror("reading failed");
    } else {
        if (aio_return(&kbcbuf) == 1) {
            c = *cp;
            mvaddch(2, 2, c);
            switch (c) {
                case 'q': case EOF:
                    //TODO: Actually handle quitting
                    player.score = nonPlayer.score = WIN_COND;
                    score = true;
                    break;
                case 'w':
                    player_move = -1;
                    break;
                case 's':
                    player_move = 1;
                    break;
            }
        }
    }
    aio_read(&kbcbuf);
}

void update(int signum) {
    mvprintw(2, (COLS/2)-2, "%i", nonPlayer.score);
    mvprintw(2, (COLS/2)+2, "%i", player.score);
    update_paddles();
    mvaddch(ball.y_pos, ball.x_pos, (ball.x_pos == COLS/2 ? ACS_VLINE : BLANK));
    ball.x_pos += ball.x_dir;
    ball.y_pos += ball.y_dir;

    if (ball.y_pos == 1 || ball.y_pos == LINES - 2) {
        ball.y_dir = -ball.y_dir;
    }
    if ((ball.x_pos == nonPlayer.x_pos+1 && (ball.y_pos >= nonPlayer.y_pos && ball.y_pos <= nonPlayer.y_pos+7)) ||
        (ball.x_pos == player.x_pos-1 && (ball.y_pos >= player.y_pos && ball.y_pos <= player.y_pos+7))) {
        ball.x_dir *= -1;
    }
    if (ball.x_pos == 1) {
        score = true;
        player.score++;
    }
    if (ball.x_pos == COLS - 2) {
        score = true;
        nonPlayer.score++;
    }
    if (!score) {
        mvaddch(ball.y_pos, ball.x_pos, ball.symbol);
    }
}

void update_paddles() {
    //TODO: Real AI
    mvvline(1, nonPlayer.x_pos, BLANK, LINES-2);
    nonPlayer.y_pos = ball.y_pos - 3;
    if (nonPlayer.y_pos <= 0) {
        nonPlayer.y_pos = 1;
    } else if (nonPlayer.y_pos+6 >= LINES-2) {
        nonPlayer.y_pos = LINES-8;
    }
    mvvline(nonPlayer.y_pos, nonPlayer.x_pos, ACS_VLINE, 7);

    //TODO: Real input
    mvvline(1, player.x_pos, BLANK, LINES-2);
    player.y_pos += player_move;
    if (player.y_pos <= 0) {
        player.y_pos = 1;
    } else if (player.y_pos+6 >= LINES-2) {
        player.y_pos = LINES-8;
    }
    mvvline(player.y_pos, player.x_pos, ACS_VLINE, 7);
    player_move = 0;
}

void cleanup() {
    attroff(COLOR_PAIR(1));
    set_ticker(0);
    endwin();
}

void add_border() {
    mvvline(0, 0, ACS_VLINE, LINES);
    mvvline(0, COLS-1, ACS_VLINE, LINES);
    mvhline(0, 0, ACS_HLINE, COLS);
    mvhline(LINES-1, 0, ACS_HLINE, COLS);
    mvaddch(0,0, ACS_ULCORNER);
    mvaddch(0, COLS-1, ACS_URCORNER);
    mvaddch(LINES-1, 0, ACS_LLCORNER);
    mvaddch(LINES-1, COLS-1, ACS_LRCORNER);
}

void setup_aio_buffer() {
    static char input[1];

    kbcbuf.aio_fildes = STDIN_FILENO;
    kbcbuf.aio_buf = input;
    kbcbuf.aio_nbytes = 1;
    kbcbuf.aio_offset = 0;

    kbcbuf.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    kbcbuf.aio_sigevent.sigev_signo = SIGIO;
}