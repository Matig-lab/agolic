CC=gcc
BIN=agolic
CFLAGS=-Wall -Wextra -Werror -pedantic
LNFLAGS=-lm -lSDL2
SRC=node.c point.c golstate.c gameoflife.c
OBJS=$(SRC:.c=.o)

all: $(BIN)

$(BIN): $(OBJS)
	gcc -o $@ $? $(CFLAGS) $(LNFLAGS)

%.o: %.c
	gcc -c $? $(CFLAGS) $(LNFLAGS)

clean:
	rm $(BIN) $(OBJS)
