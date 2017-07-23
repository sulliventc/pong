Pong: pong.c pong.h set_ticker.c set_ticker.h
	gcc -Wall -g pong.c pong.h set_ticker.c set_ticker.h -lrt -o Pong -lncurses
