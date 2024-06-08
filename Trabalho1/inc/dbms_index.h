#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>


// Estrutura que guarda dados do index
typedef struct
{
    void** key;
    int64_t* byte_offset;
    int key_size;
    int key_row;
    int num_reg;
} Index;


void index_free(Index* index);