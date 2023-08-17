CC=gcc
BIN=agolic
CFLAGS=-Wall -Wextra -Werror -pedantic
LNFLAGS=-lm -lSDL2
SRC=node.c point.c golstate.c gui.c main.c
OBJS=$(SRC:.c=.o)

TESTS_DIR=tests
TESTS_SRC=$(wildcard $(TESTS_DIR)/*.c)
TESTS_BINS=$(patsubst $(TESTS_DIR)/%.c, $(TESTS_DIR)/bin/%, $(TESTS_SRC))

all: $(BIN)

$(BIN): $(OBJS)
	gcc -o $@ $? $(CFLAGS) $(LNFLAGS)

%.o: %.c
	gcc -c $? $(CFLAGS) $(LNFLAGS)

$(BIN_TESTS): $(TESTS_BINS)

$(TESTS_BINS): $(TESTS_DIR)/bin/% : $(TESTS_DIR)/%.c $(OBJS)
	$(CC) -o $@ $< $(CFLAGS) $(filter-out main.o, $(OBJS)) $(LNFLAGS) -lcriterion

test: $(TESTS_BINS)
	for test in $(TESTS_BINS) ; do ./$$test ; done

test_verbose: $(TESTS_BINS)
	for test in $(TESTS_BINS) ; do ./$$test --verbose ; done

clean:
	rm $(BIN) $(OBJS)
