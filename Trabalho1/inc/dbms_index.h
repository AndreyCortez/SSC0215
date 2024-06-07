#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "dbms_table.h"

// Estrutura que guarda dados do index
typedef struct
{
    void** key;
    int64_t* byte_offset;
    int key_size;
    int key_row;
} Index;


bool table_create_index(Table *table, char* path, int key_row, int key_size);
bool table_load_index(Table *table, char *path, int key_row, int key_size);