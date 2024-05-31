#ifndef CSV_UTILS
#define CSV_UTILS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 2 << 16
#define MAX_FIELD_LENGTH 2 << 16

typedef struct
{
    char ***data;
    int num_rows;
    int num_collumns;
    char **header;
} CSV_handler;

// Abre o arquivo csv e salva seus dados em uma estrutura adequada para 
// manipulação por código
CSV_handler *csv_parse(FILE *file, bool has_header);

// Libera a memoria usada pela estrutura quando a mesma não for mais
// necessária
void csv_free_handle(CSV_handler **handler);

// Printa as primeiras 5 linhas do csv
// util para debug
void csv_print_head(CSV_handler *handler);

// Acha o indice da coluna de tal nome
int csv_find_collumn(CSV_handler *handler, char *header);

#endif