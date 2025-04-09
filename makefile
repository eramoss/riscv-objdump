CC = clang
CFLAGS = -Wall -Werror -Iinclude

SRC_DIR = src
OBJ_DIR = build/obj
BUILD_DIR = build
LIB_NAME = libriscvdump.a
MAIN = $(SRC_DIR)/main.c 

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

all: clean main

compile_lib: $(BUILD_DIR)/$(LIB_NAME)

$(BUILD_DIR)/$(LIB_NAME): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	@echo "Creating static library $@"
	ar rcs $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling $< to $@"
	$(CC) $(CFLAGS) -c $< -o $@

main: compile_lib $(MAIN)
	@echo "Compiling main program..."
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/riscv-objdump $(MAIN) -L$(BUILD_DIR)/ -lriscvdump

clean:
	@echo "Cleaning up..."
	rm -f $(OBJ_DIR)/*.o $(BUILD_DIR)/$(LIB_NAME) $(BUILD_DIR)/riscv-objdump
	@rmdir $(OBJ_DIR) 2>/dev/null || true
	@rmdir $(BUILD_DIR) 2>/dev/null || true

.PHONY: all compile_lib main clean
