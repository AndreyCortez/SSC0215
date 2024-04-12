CC = gcc
CFLAGS = -Wall -Wextra -Iinc
SRC_DIR = src
INC_DIR = inc
BIN_DIR = bin
TARGET = myprogram

# Lista de arquivos fonte
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# Lista de arquivos objeto
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRC_FILES))

all: $(BIN_DIR)/$(TARGET)

# Compilar os arquivos objeto
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Linkar os objetos para gerar o executável
$(BIN_DIR)/$(TARGET): $(OBJ_FILES)
	$(CC) $^ -o $@

# Executar o programa
run:
	./$(BIN_DIR)/$(TARGET)

# Limpar os arquivos objeto e o executável
clean:
	rm -f $(BIN_DIR)/*
