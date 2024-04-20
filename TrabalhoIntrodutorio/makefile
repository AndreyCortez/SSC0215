CC = gcc
CFLAGS = -Wall -Wextra -Iinc
SRC_DIR = src
INC_DIR = inc
BIN_DIR = bin
TARGET = programaTrab

# Lista de arquivos fonte
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# Lista de arquivos objeto
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRC_FILES))

all: $(TARGET)

# Compilar os arquivos objeto
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -ggdb3

# Linkar os objetos para gerar o executável
$(TARGET): $(OBJ_FILES)
	$(CC) $^ -o $@

# Executar o programa
run:
	./$(TARGET)

check_memory:
	valgrind --leak-check=full ./programaTrab

# Limpar os arquivos objeto e o executável
clean:
	rm -f $(BIN_DIR)/*

