#ifndef DBMS
#define DBMS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "csv.h"

typedef struct
{
    char status;
    int64_t top;
    int64_t next_byte_offset;
    int32_t num_reg;
    int32_t num_removed;
    char *format;
    int num_fields;
    int field_types;
} TableHandler;


typedef struct
{
    char removed;
    int32_t tam_reg;
    int64_t prox_reg;
    void *data;
} RegHeader;

typedef struct
{
    void **data;
    int data_size;
    TableHandler *handler;
    RegHeader *register_headers;
} Table;


Table *table_create(char ***raw_data, char *format, int num_rows, int num_collumns);
Table *table_create_from_csv(CSV_handler* handler, char *format);

void table_save(Table *table, char* path);
TableHandler *table_read(char *path, char *format);
// char table_seek(char);

void *add_raw_item(Table *Table, void *raw_item);
void *format_data(const char *format, char **data);
int format_len(const char *format, char **data);

#endif