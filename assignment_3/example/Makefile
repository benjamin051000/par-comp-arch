# Mostly from makefiletutorial.com
EXEC := matmult
CFLAGS := -O0 -Wall -Wextra -Wunused -Wshadow -pedantic -Wwrite-strings
ARGS := ""
CC := "mpicc"

BUILD_DIR := ./build
SRC_DIRS := ./src
INCLUDE_DIRS := .

# Find C files to compile
# Note the single quotes around the * expressions. Make will incorrectly expand these otherwise.
SRCS := $(shell find $(SRC_DIRS) -name '*.c')

# String substitution for every C file.
# For example, hello.c turns into ./build/hello.o
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)

# The final build step.
$(EXEC): $(OBJS)
	$(CC) -I $(INCLUDE_DIRS) $(CFLAGS) $(OBJS) -o $@

# Build step for C source
$(BUILD_DIR)/%.o: $(SRCS)
	mkdir -p $(dir $@)
	$(CC) -I $(INCLUDE_DIRS) $(CFLAGS) -c $< -o $@

.PHONY: clean run
clean:
	rm -rf $(BUILD_DIR) $(EXEC)
	rm -f myoutput myerr out.txt err.txt

run: $(EXEC)
	sbatch ./job.sh

