#ifndef DBMS
#define DBMS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct
{
    void **data;
    int data_size;
    int num_fields;
    int field_types;
} table;

table *table_create(void **raw_data, char *format);
void *add_raw_item(table *table, void *raw_item);
void *format_data(const char *format, char **data);
int format_len(const char *format, char **data);

#endif