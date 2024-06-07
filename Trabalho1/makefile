CC = gcc
CFLAGS = -Iinc -g -Wall -Wextra  -std='c99' #-pedantic
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
	$(CC) $(CFLAGS) -c $< -o $@ 

# Linkar os objetos para gerar o executável
$(TARGET): $(OBJ_FILES)
	$(CC) $^ -o $@ 

# Executar o programa
run:
	./$(TARGET)

check_memory:
	valgrind --leak-check=full ./programaTrab -s

# Limpar os arquivos objeto e o executável
clean:
	rm -f $(BIN_DIR)/*

