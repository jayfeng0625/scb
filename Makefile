CC      ?= cc
AR      ?= ar
CFLAGS  := -std=c23 -Wall -Wextra -Wpedantic -Werror -Iinclude
LDFLAGS :=

SRC_DIR   := src
CLI_DIR   := cli
TEST_DIR  := tests
BUILD_DIR := build

LIB_SRC := $(wildcard $(SRC_DIR)/*.c)
CLI_SRC := $(wildcard $(CLI_DIR)/*.c)
TEST_SRC := $(wildcard $(TEST_DIR)/*.c)

LIB_OBJ  := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(LIB_SRC))
CLI_OBJ  := $(patsubst $(CLI_DIR)/%.c, $(BUILD_DIR)/cli_%.o, $(CLI_SRC))
TEST_OBJ := $(patsubst $(TEST_DIR)/%.c, $(BUILD_DIR)/test_%.o, $(TEST_SRC))

LIB  := $(BUILD_DIR)/libchess.a
BIN  := $(BUILD_DIR)/chess
TEST := $(BUILD_DIR)/test_chess

all: $(BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/cli_%.o: $(CLI_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/test_%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(LIB_OBJ)
	$(AR) rcs $@ $^

$(BIN): $(CLI_OBJ) $(LIB)
	$(CC) $(CLI_OBJ) -L$(BUILD_DIR) -lchess $(LDFLAGS) -o $@

$(TEST): $(TEST_OBJ) $(LIB)
	$(CC) $(TEST_OBJ) -L$(BUILD_DIR) -lchess $(LDFLAGS) -o $@

test: $(TEST)
	./$(TEST)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test clean
