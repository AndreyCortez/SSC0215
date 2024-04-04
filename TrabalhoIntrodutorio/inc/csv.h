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
} CSV_handler;

CSV_handler *csv_parse(FILE *file, bool skip_first_line);
void csv_print_head(CSV_handler* handler);


#endif