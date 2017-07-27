#include "pong.h"

/*
 * Main function
 * pass control to other functions
 */
int main(int argc, char *argv[]) {
    setup_ncurses();
    main_menu();
    cleanup_ncurses();

    return 0;
}

/*
 * Setup ncurses
 */
void setup_ncurses() {
    g_ai_speed_limiter = 0;                              // Used to limit movement speed of AI paddle
    g_ai_fudge = 0;

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
 * Leave it like you found it
 */
void cleanup_ncurses() {
    attroff(COLOR_PAIR(1));
    set_ticker(0);
    endwin();
}

/*
 * Main menu
 * any key moves to difficulty selection
 */
void main_menu() {
    int text_width = 44;
    int text_line = (LINES-10)/2;
    clear();
    add_border();
    mvprintw(text_line++,  (COLS-text_width)/2, "$$$$$$$\\   $$$$$$\\  $$\\   $$\\  $$$$$$\\  $$\\ ");
    mvprintw(text_line++,  (COLS-text_width)/2, "$$  __$$\\ $$  __$$\\ $$$\\  $$ |$$  __$$\\ $$ |");
    mvprintw(text_line++,  (COLS-text_width)/2, "$$ |  $$ |$$ /  $$ |$$$$\\ $$ |$$ /  \\__|$$ |");
    mvprintw(text_line++,  (COLS-text_width)/2, "$$$$$$$  |$$ |  $$ |$$ $$\\$$ |$$ |$$$$\\ $$ |");
    mvprintw(text_line++,  (COLS-text_width)/2, "$$  ____/ $$ |  $$ |$$ \\$$$$ |$$ |\\_$$ |\\__|");
    mvprintw(text_line++,  (COLS-text_width)/2, "$$ |      $$ |  $$ |$$ |\\$$$ |$$ |  $$ |    ");
    mvprintw(text_line++, (COLS-text_width)/2, "$$ |       $$$$$$  |$$ | \\$$ |\\$$$$$$  |$$\\ ");
    mvprintw(text_line++, (COLS-text_width)/2, "\\__|       \\______/ \\__|  \\__| \\______/ \\__|");
    mvprintw(text_line++, (COLS-text_width)/2, "Control with w for UP and s for DOWN");
    mvprintw(text_line++, (COLS-text_width)/2, "Press any key to start!");

    // TODO: Restrict play area properly
    if (COLS > 80 || LINES > 24) {
        mvprintw(LINES-4, (COLS/2)-28, "NOTE: This game intended for smaller play area.");
        mvprintw(LINES-3, (COLS/2)-28, "      Recommended dimensions: 80 x 24");
    }

    refresh();
    getch();
    difficulty_menu();
}

/*
 * Difficulty selection screen
 * recurses on bad input
 */
void difficulty_menu() {
    int c;
    int text_width = 30;
    int text_line = (LINES-4)/2;
    clear();
    add_border();

    mvprintw(text_line++, (COLS-text_width)/2, "Select Ball Speed:");
    mvprintw(text_line++, (COLS-text_width)/2, "1. Easy - Slow Ball Speed");
    mvprintw(text_line++, (COLS-text_width)/2, "2. Medium - Normal Ball Speed");
    mvprintw(text_line++, (COLS-text_width)/2, "3. Hard - Fast Ball Speed");

    refresh();
    c = getch();
    // TODO: make g_diff local
    switch (c) {
        case '1': g_diff = DIFF_EASY; game_loop(); break;
        case '2': g_diff = DIFF_NORM; game_loop(); break;
        case '3': g_diff = DIFF_HARD; game_loop(); break;
        default: difficulty_menu();
    }
}

/*
 * Display pause menu to allow option selection
 * Rescurse on bad input
 */
void pause_menu() {
    set_ticker(0);                                  // Stop play

    int c;
    int text_width = 18;
    int text_line = (LINES-4)/2;

    clear();
    add_border();

    mvprintw(text_line++, (COLS-text_width)/2, "Game Paused:");
    mvprintw(text_line++, (COLS-text_width)/2, "1. Return to game");
    mvprintw(text_line++, (COLS-text_width)/2, "2. Reset game");
    mvprintw(text_line++, (COLS-text_width)/2, "3. Quit");

    refresh();
    c = getch();
    switch (c) {
        case '2':
            g_player.score = g_non_player.score = 0;        // Reset
        case '1':                                           // Return to game
            set_ticker(g_diff);
            clear();
            add_border();
            mvvline(0, COLS / 2, ACS_VLINE, LINES);
            mvaddch(0, COLS / 2, ACS_TTEE);
            mvaddch(LINES - 1, COLS / 2, ACS_BTEE);
            break;
        case '3':
            g_player.score = g_non_player.score = WIN_COND;
            g_score = true;
            break; // Quit
        default:
            pause_menu();
    }
}

/*
 * Last minute setup_ncurses and
 * main game loop w/ inner
 * refresh loop.
 */
void game_loop() {
    time_t t;
    int dir;

    clear();
    add_border();
    mvvline(0, COLS / 2, ACS_VLINE, LINES); // add mid-bar
    mvaddch(0, COLS / 2, ACS_TTEE);         // Mid-bar junction
    mvaddch(LINES-1, COLS / 2, ACS_BTEE);   // Mid-bar junction

    setup_aio_buffer();
    aio_read(&g_kbcbuf);
    signal(SIGIO, on_input);
    signal(SIGALRM, update);
    set_ticker(g_diff);

    // TODO: Reevaluate whatever life choices made you love globals so much
    g_non_player.x_pos = 2;
    g_player.x_pos = COLS-3;
    g_player.score = g_non_player.score = 0;
    g_ball.x_dir = g_ball.y_dir = 1;
    g_ball.symbol = SYM_BAL;
    srand((unsigned) time(&t));

    /*
     * main loop
     * continues until winner
     */
    while (g_player.score != WIN_COND && g_non_player.score != WIN_COND) {
        g_ball.x_pos = COLS/2;
        g_ball.y_pos = LINES/2;
        dir = rand() % 4;           // Randomly decide ball direction
        switch (dir) {
            case 0:                 // Up, Left
                g_ball.x_dir = -1;
                g_ball.y_dir = -1;
                break;
            case 1:                 // Down, Left
                g_ball.x_dir = -1;
                g_ball.y_dir = 1;
                break;
            case 2:                 // Down, Right
                g_ball.x_dir = 1;
                g_ball.y_dir = 1;
                break;
            case 3:                 // Up, Right
                g_ball.x_dir = 1;
                g_ball.y_dir = -1;
        }

        g_non_player.y_pos = (LINES/2) - 2;   // Reset paddle positions
        g_player.y_pos = (LINES/2) - 2;

        g_score = false;
        while (!g_score) {
            refresh();
        }
        // TODO: Game over screen
    }
}

/*
 * SIGIO handler
 */
void on_input(int signum) {
    int c;
    char *cp = (char *) g_kbcbuf.aio_buf;

    if (aio_error(&g_kbcbuf) != 0) {
        perror("reading failed");
    } else {
        if (aio_return(&g_kbcbuf) == 1) {
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
    aio_read(&g_kbcbuf);
}

/*
 * update ball location, handle bounce or score logic
 * calls update_paddles()
 */
void update(int signum) {
    mvprintw(2, (COLS/2)-2, "%i", g_non_player.score); // Display score
    mvprintw(2, (COLS/2)+2, "%i", g_player.score);     // Display score
    update_paddles();
    mvaddch(g_ball.y_pos, g_ball.x_pos, (g_ball.x_pos == COLS/2 ? ACS_VLINE : BLANK)); // Clear ball or re-add midline
    g_ball.x_pos += g_ball.x_dir;
    g_ball.y_pos += g_ball.y_dir;

    /*
     * bounce or score logic
     */
    if (g_ball.y_pos == 1 || g_ball.y_pos == LINES - 2) { // Hit top or bottom
        g_ball.y_dir = -g_ball.y_dir;
    }
    if ((g_ball.x_pos == g_non_player.x_pos+1 && (g_ball.y_pos >= g_non_player.y_pos && g_ball.y_pos <= g_non_player.y_pos+4)) ||
        (g_ball.x_pos == g_player.x_pos-1 && (g_ball.y_pos >= g_player.y_pos && g_ball.y_pos <= g_player.y_pos+6))) {               // Hit paddle
        g_ball.x_dir *= -1;
    }
    if (g_ball.x_pos == 1) { // Player scored
        g_score = true;
        g_player.score++;
    }
    if (g_ball.x_pos == COLS - 2) { // AI scored
        g_score = true;
        g_non_player.score++;
    }
    if (!g_score) { // No bounce, no score, move the ball
        mvaddch(g_ball.y_pos, g_ball.x_pos, g_ball.symbol);
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
    if (g_ball.x_dir == 1) {                                                // Disable ai while ball is moving toward player
        g_ai_speed_limiter = 0;                                             // zeroing limiter also allows for ai reaction delay

        int fudge_number = (rand() % 3) + 1;                                // generate ai fudge number
        int fudge_sign = rand() % 2;                                        // yes, every frame the ball is moving away from ai
        g_ai_fudge = fudge_number * (fudge_sign == 0 ? -1 : 1);             // TODO: Move this block to somewhere not dumb
    } else {
        g_ai_speed_limiter++;
    }
    mvvline(1, g_non_player.x_pos, BLANK, LINES - 2);                       // Clear paddle
    if (g_ai_speed_limiter % 5 == 0 && g_ai_speed_limiter > g_diff / 4) {   // move after short pause, then limit speed
        ai_y_target = calculate_ai_target() + g_ai_fudge;                   // Simulate ball, move to target
        if (g_non_player.y_pos > ai_y_target - 2) {
            g_non_player.y_pos--;
        } else if (g_non_player.y_pos < ai_y_target - 2) {
            g_non_player.y_pos++;
        }
        if (g_non_player.y_pos <= 0) {                                      // paddle went past top line
            g_non_player.y_pos = 1;
        } else if (g_non_player.y_pos + 5 >= LINES - 2) {                   // paddle went past bottom line
            g_non_player.y_pos = LINES - 6;
        }
    }
    mvvline(g_non_player.y_pos, g_non_player.x_pos, ACS_VLINE, 5);          // Redraw paddle

    /*
     * User movement handling
     */
    mvvline(1, g_player.x_pos, BLANK, LINES-2);                   // Clear paddle
    g_player.y_pos += g_player_move;                              // No pause for player
    if (g_player.y_pos <= 0) {                                    // paddle went past top line
        g_player.y_pos = 1;
    } else if (g_player.y_pos+6 >= LINES-2) {                     // paddle went past bottom line
        g_player.y_pos = LINES-8;
    }
    mvvline(g_player.y_pos, g_player.x_pos, ACS_VLINE, 7);        // Redraw paddle
    g_player_move = 0;                                            // Reset player movement modifier
}

/*
 * Simulate ball to get ai target.
 */
int calculate_ai_target() {
    struct p_ball sim = g_ball;
    while (sim.x_pos != g_non_player.x_pos) {
        sim.x_pos += sim.x_dir;
        sim.y_pos += sim.y_dir;

        if (sim.y_pos == 1 || sim.y_pos == LINES - 2) {
            sim.y_dir = -sim.y_dir;
        }
    }
    return sim.y_pos;
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

    g_kbcbuf.aio_fildes = STDIN_FILENO;
    g_kbcbuf.aio_buf = input;
    g_kbcbuf.aio_nbytes = 1;
    g_kbcbuf.aio_offset = 0;

    g_kbcbuf.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    g_kbcbuf.aio_sigevent.sigev_signo = SIGIO;
}