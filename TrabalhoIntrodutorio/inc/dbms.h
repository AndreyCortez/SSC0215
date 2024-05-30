#ifndef DBMS
#define DBMS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "csv.h"

#define register_header_size 13
#define table_header_size 25

// Estrutura que guarda dados do registro
typedef struct
{
    char removed;
    int32_t tam_reg;
    int64_t prox_reg;
    void *data;
} RegHeader;

// Estrutura que guardad dados da tabela
typedef struct
{
    void **data;
    int data_size;

    char status;
    int64_t top;
    int64_t next_byte_offset;
    int32_t num_reg;
    int32_t pos_reg;
    int32_t num_removed;
    char *format;
    int num_fields;
    int field_types;
    FILE *f_pointer;
    RegHeader current_register;
    RegHeader *register_headers;
} Table;

// Função para criar uma tabela a partir de dados especificos
Table *table_create(char ***raw_data, char *format, int num_rows, int num_collumns);

// Função para criar uma tabela a partir de uma handler de csv
Table *table_create_from_csv(CSV_handler *handler, char *format);

// Salva a tabela na memória como um binário
void table_save(Table *table, char *path);

// Acessa a tabela na memória
// NOTA: não carrega o campo data
Table *table_access(char *path);

// Move o ponteiro de registro da tabela para o proximo registro
bool table_move_to_next_register(Table *table);
// Move o ponteiro de registro da tabela para o primeiro registro
void table_reset_register_pointer(Table *table);
// Libera a memoria utilizada pela tabela
void table_free(Table **tab);

// Funções auxiliares para a criação da tabela
void *format_data(const char *format, char **data);
int format_len(const char *format, char **data);

#endif