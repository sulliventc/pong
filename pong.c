#include "pong.h"

/*
 * Main function
 * pass control to other functions
 */
int main(int argc, char *argv[]) {
    setup();
    menu();
    cleanup();

    return 0;
}

/*
 * Setup ncurses
 */
void setup() {
    g_limiter = 0;                              // Used to limit movement speed of AI paddle

    initscr();
    if (COLS < 80 || LINES < 24) {
        mvprintw(0,0,"Play area too small.");
        mvprintw(1,0,"Set to 80 x 24");
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

/*
 * Main menu
 * any key moves to difficulty selection
 */
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
    mvprintw(13, (COLS/2)-22, "Control with w for UP and s for DOWN");
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

/*
 * Difficulty selection screen
 * recurses on bad input
 */
void difficulty() {
    int c;
    clear();
    add_border();

    mvprintw(4, (COLS/2)-22, "Select Ball Speed:");
    mvprintw(6, (COLS/2)-22, "1. Easy - Slow Ball Speed");
    mvprintw(8, (COLS/2)-22, "2. Medium - Normal Ball Speed");
    mvprintw(10, (COLS/2)-22, "3. Hard - Fast Ball Speed");

    refresh();
    c = getch() - '0';
    if (c > 0 && c < 4) {
        switch (c) {
            case 1: g_diff = DIFF_EASY; break;
            case 2: g_diff = DIFF_NORM; break;
            case 3: g_diff = DIFF_HARD; break;
        }
        game();
    } else {
        difficulty();
    }
}

/*
 * Last minute setup and
 * main game loop w/ inner
 * refresh loop.
 */
void game() {
    time_t t;
    int dir;

    clear();
    add_border();
    mvvline(0, COLS / 2, ACS_VLINE, LINES); // add mid-bar
    mvaddch(0, COLS / 2, ACS_TTEE);         // Mid-bar junction
    mvaddch(LINES-1, COLS / 2, ACS_BTEE);   // Mid-bar junction

    setup_aio_buffer();
    aio_read(&kbcbuf);
    signal(SIGIO, on_input);
    signal(SIGALRM, update);
    set_ticker(g_diff);

    non_player.x_pos = 2;
    player.x_pos = COLS-3;
    player.score = non_player.score = 0;
    ball.x_dir = ball.y_dir = 1;
    ball.symbol = SYM_BAL;
    srand((unsigned) time(&t));

    /*
     * main loop
     * continues until winner
     */
    while (player.score != WIN_COND && non_player.score != WIN_COND) {
        g_score = false;

        ball.x_pos = COLS/2;
        ball.y_pos = LINES/2;
        dir = rand() % 4;           // Randomly decide ball direction
        switch (dir) {
            case 0:                 // Up, Left
                ball.x_dir = -1;
                ball.y_dir = -1;
                break;
            case 1:                 // Down, Left
                ball.x_dir = -1;
                ball.y_dir = 1;
                break;
            case 2:                 // Down, Right
                ball.x_dir = 1;
                ball.y_dir = 1;
                break;
            case 3:                 // Up, Right
                ball.x_dir = 1;
                ball.y_dir = -1;
        }

        non_player.y_pos = (LINES/2) - 2;   // Reset paddle positions
        player.y_pos = (LINES/2) - 2;

        while (!g_score) {
            refresh();
        }
        //TODO: Game over screen
    }
}

/*
 * Display pause menu to allow option selection
 * Rescurse on bad input
 */
void pause_menu() {
    set_ticker(0);                                  // Stop play

    int c;

    clear();
    add_border();

    mvprintw(4, (COLS/2)-22, "Game Paused:");
    mvprintw(6, (COLS/2)-22, "1. Return to game");
    mvprintw(8, (COLS/2)-22, "2. Reset game");
    mvprintw(10, (COLS/2)-22, "3. Quit");

    refresh();
    c = getch() - '0';
    if (c > 0 && c < 4) {
        switch (c) {
            case 2: player.score = non_player.score = 0;        // Reset
            case 1:                                             // Return to game
                set_ticker(g_diff);
                clear();
                add_border();
                mvvline(0, COLS / 2, ACS_VLINE, LINES);
                mvaddch(0, COLS / 2, ACS_TTEE);
                mvaddch(LINES-1, COLS / 2, ACS_BTEE);
                break;
            case 3: player.score = non_player.score = WIN_COND; g_score = true; break; // Quit
        }
    } else {
        pause_menu();                                           // Recurse on bad input
    }
}

/*
 * SIGIO handler
 */
void on_input(int signum) {
    int c;
    char *cp = (char *) kbcbuf.aio_buf;

    if (aio_error(&kbcbuf) != 0) {
        perror("reading failed");
    } else {
        if (aio_return(&kbcbuf) == 1) {
            c = *cp;
            switch (c) {
                case 'q': case EOF:
                    pause_menu();
                    break;
                case 'w':
                    g_player_move = -1;
                    break;
                case 's':
                    g_player_move = 1;
                    break;
            }
        }
    }
    aio_read(&kbcbuf);
}

/*
 * update ball location, handle bounce or score logic
 * calls update_paddles()
 */
void update(int signum) {
    mvprintw(2, (COLS/2)-2, "%i", non_player.score); // Display score
    mvprintw(2, (COLS/2)+2, "%i", player.score);     // Display score
    update_paddles();
    mvaddch(ball.y_pos, ball.x_pos, (ball.x_pos == COLS/2 ? ACS_VLINE : BLANK)); // Clear ball or re-add midline
    ball.x_pos += ball.x_dir;
    ball.y_pos += ball.y_dir;

    /*
     * bounce or score logic
     */
    if (ball.y_pos == 1 || ball.y_pos == LINES - 2) { // Hit top or bottom
        ball.y_dir = -ball.y_dir;
    }
    if ((ball.x_pos == non_player.x_pos+1 && (ball.y_pos >= non_player.y_pos && ball.y_pos <= non_player.y_pos+4)) ||
        (ball.x_pos == player.x_pos-1 && (ball.y_pos >= player.y_pos && ball.y_pos <= player.y_pos+6))) {               // Hit paddle
        ball.x_dir *= -1;
    }
    if (ball.x_pos == 1) { // Player scored
        g_score = true;
        player.score++;
    }
    if (ball.x_pos == COLS - 2) { // AI scored
        g_score = true;
        non_player.score++;
    }
    if (!g_score) { // No bounce, no score, move the ball
        mvaddch(ball.y_pos, ball.x_pos, ball.symbol);
    }
}

/*
 * Called from update(int) to move paddles
 * at set intervals.
 */
void update_paddles() {
    int ai_y_target;
    /*
     * AI movement handling
     */
    if (ball.x_dir == 1) {
        g_limiter = 0;
    } else {
        g_limiter++;
    }
    mvvline(1, non_player.x_pos, BLANK, LINES - 2);             // Clear paddle
    if (g_limiter % 5 == 0 && g_limiter > g_diff / 4) {         // move after short pause, then limit speed
        ai_y_target = ai_target();                              // Simulate ball, move to target
        //TODO: Fudge simulation by random amount.
        if (non_player.y_pos > ai_y_target - 2) {
            non_player.y_pos--;
        } else if (non_player.y_pos < ai_y_target - 2) {
            non_player.y_pos++;
        }
        if (non_player.y_pos <= 0) {                            // y_pos went past top line
            non_player.y_pos = 1;
        } else if (non_player.y_pos + 5 >= LINES - 2) {         // y_pos went past bottom line
            non_player.y_pos = LINES - 6;
        }
    }
    mvvline(non_player.y_pos, non_player.x_pos, ACS_VLINE, 5);  // Redraw paddle

    /*
     * User movement handling
     */
    mvvline(1, player.x_pos, BLANK, LINES-2);                   // Clear paddle
    player.y_pos += g_player_move;                              // No pause for player
    if (player.y_pos <= 0) {                                    // y_pos went past top line
        player.y_pos = 1;
    } else if (player.y_pos+6 >= LINES-2) {                     // y_pos went past bottom line
        player.y_pos = LINES-8;
    }
    mvvline(player.y_pos, player.x_pos, ACS_VLINE, 7);          // Redraw paddle
    g_player_move = 0;                                          // Reset player movement modifier
}

/*
 * Simulate ball to get ai target.
 */
int ai_target() {
    struct p_ball sim = ball;
    while (sim.x_pos != non_player.x_pos) {
        sim.x_pos += sim.x_dir;
        sim.y_pos += sim.y_dir;

        if (sim.y_pos == 1 || sim.y_pos == LINES - 2) {
            sim.y_dir = -sim.y_dir;
        }
    }
    return sim.y_pos;
}

/*
 * Leave it like you found it
 */
void cleanup() {
    attroff(COLOR_PAIR(1));
    set_ticker(0);
    endwin();
}

/*
 * Make it pretty
 */
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

/*
 * Does what it says on the tin
 */
void setup_aio_buffer() {
    static char input[1];

    kbcbuf.aio_fildes = STDIN_FILENO;
    kbcbuf.aio_buf = input;
    kbcbuf.aio_nbytes = 1;
    kbcbuf.aio_offset = 0;

    kbcbuf.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    kbcbuf.aio_sigevent.sigev_signo = SIGIO;
}