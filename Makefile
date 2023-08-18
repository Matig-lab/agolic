CC=gcc
BIN=agolic
CFLAGS=-Wall -Wextra -Werror -pedantic
LNFLAGS=-lm -lSDL2

SRC_DIR=src
SRC=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))

BUILD_DIR=build
BUILD_DIR_CREATED=

TESTS_DIR=tests
TESTS_BIN_DIR=tests/bin
TESTS_SRC=$(wildcard $(TESTS_DIR)/*.c)
TESTS_BINS=$(patsubst $(TESTS_DIR)/%.c, $(TESTS_DIR)/bin/%, $(TESTS_SRC))
TESTS_BIN_DIR_CREATED=

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LNFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@if [ -z "$(BUILD_DIR_CREATED)" ]; then \
		mkdir -p $(BUILD_DIR); \
		BUILD_DIR_CREATED=1; \
	fi
	$(CC) -c -o $@ $< $(CFLAGS)

$(TESTS_BINS): $(TESTS_DIR)/bin/% : $(TESTS_DIR)/%.c $(OBJS)
	@if [ -z "$(TEST_BIN_DIR_CREATED)" ]; then \
		mkdir -p $(TESTS_BIN_DIR); \
		TESTS_BIN_DIR_CREATED=1; \
	fi
	@echo "[*] Building $@..."
	@$(CC) -o $@ $< $(CFLAGS) $(filter-out $(BUILD_DIR)/main.o, $(OBJS)) $(LNFLAGS) -lcriterion

test: $(TESTS_BINS)
	@echo -e "[*] Running tests..."
	@for test in $(filter-out $(TESTS_DIR)/bin/perf, $(TESTS_BINS)) ; do ./$$test ; done
	@echo "[*] Done"

verbose_test: $(TESTS_BINS)
	for test in $(TESTS_BINS) ; do ./$$test --verbose ; done

performance_test: $(TESTS_BINS)
	@echo "[*] Running performance test"
	@./$(TESTS_DIR)/bin/perf --verbose
	@echo "[*] Done"

clean:
	rm -f $(BIN) $(OBJS)
