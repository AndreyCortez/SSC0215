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

CSV_handler *csv_parse(FILE *file, bool has_header);
char **csv_retrieve_collumn(CSV_handler* handler, char* collumn);
CSV_handler *csv_retrieve_collumns(CSV_handler* handler, char** collumns, int qtd_collumns);
void csv_free_handle(CSV_handler** handler);
void csv_free_collum(char **collumn);

void csv_print_head(CSV_handler* handler);
int csv_find_collumn(CSV_handler* handler, char* header);


#endif