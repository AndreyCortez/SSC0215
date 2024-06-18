#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "dbms_format.h"

#define register_header_size 13

// Estrutura que guarda dados do registro
typedef struct
{
    char removed;
    int32_t tam_reg;
    int64_t prox_reg;
    int64_t byte_offset;
    void *data;
} Register;

bool register_write(Register reg, FILE *file, char *format);
Register register_read(int64_t offset, FILE *file);
void register_free(Register *reg);