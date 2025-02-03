# Variables
CC = gcc
CFLAGS = -Wall -Wextra -O0 -fopenmp
SRC_FILES = $(wildcard *.c)
OUT_DIR = out
OUT_FILES = $(patsubst %.c,$(OUT_DIR)/%.out,$(SRC_FILES))

# Default target
all: $(OUT_DIR) $(OUT_FILES)

# Rule to compile each .c file into out/ directory
$(OUT_DIR)/%.out: %.c
	$(CC) $(CFLAGS) $< -o $@

# Create the output directory if it doesn't exist
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

# Clean target to remove all .out files and the out/ directory
clean:
	rm -rf $(OUT_DIR)

.PHONY: all clean
