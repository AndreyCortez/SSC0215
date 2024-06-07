#ifndef DBMS
#define DBMS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "csv.h"
#include "dbms_format.h"
#include "dbms_index.h"

#define register_header_size 13
#define table_header_size 25

// Estrutura que guarda dados do index
typedef struct
{
    void** key;
    int64_t* byte_offset;
    int key_size;
    int key_row;
} Index;


// Estrutura que guarda dados do registro
typedef struct
{
    char removed;
    int32_t tam_reg;
    int64_t prox_reg;
    int64_t byte_offset;
    void *data;
} Register;

// Estrutura que guarda dados da tabela
typedef struct
{
    char status;
    int64_t top;
    int64_t next_byte_offset;
    int32_t num_reg;
    int32_t num_removed;

    char *format;
    FILE *f_pointer;
    
    Register current_register;
    
    Index index;
    bool index_loaded;
} Table;



// Função para criar uma tabela a partir de dados especificos
Table *table_create(char* path, char ***raw_data, char *format, int num_rows);

// Função para criar uma tabela a partir de uma handler de csv
Table *table_create_from_csv(char* path, CSV_handler *handler, char *format);

// Salva a tabela na memória como um binário
bool table_save(Table *table, char *path);

// Acessa a tabela na memória
// NOTA: não carrega o campo data
Table *table_access(char *path, char* format);


bool table_append_register(Table *table, Register reg);
// Move o ponteiro de registro da tabela para o proximo registro
bool table_move_to_next_register(Table *table);
// Move o ponteiro de registro da tabela para o primeiro registro
void table_reset_register_pointer(Table *table);

bool table_delete_current_register(Table *table);

bool table_search_for_matches(Table *table, void** data, int* indexes, int num_parameters);

// Libera a memoria utilizada pela tabela
void table_free(Table **tab);

bool table_insert_new_register(Table* table, char** row);


bool table_create_index(Table *table, char* path, int key_row, int key_size);
bool table_load_index(Table *table, char *path, int key_row, int key_size);

#endif