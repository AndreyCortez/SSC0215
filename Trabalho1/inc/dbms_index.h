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
} Index;

// Esse arquivo não possui funções mas foi deixado aqui para 
// quando houverem implementações futuras envolvendo arvore B
