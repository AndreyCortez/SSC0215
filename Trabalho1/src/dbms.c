#include "dbms.h"

// Funções auxiliares para a criação da tabela
void *format_data(char *format, char **data);
int format_len(char *format, char **data);
char *decode_format(char *format);

Table *table_create(char ***raw_data, char *format, int num_rows, int num_collumns)
{
    Table *table;
    table = (Table *)malloc(sizeof(Table));

    table->format = decode_format(format);
    table->data = (void **)malloc(sizeof(void *) * num_rows);
    table->data_size = num_rows;
    table->register_headers = (Register *)malloc(sizeof(Register) * num_rows);

    uint64_t next_byte_offset = table_header_size;

    for (int i = 0; i < num_rows; i++)
    {
        // printf("%s\n", table->format);
        table->data[i] = format_data(table->format, raw_data[i]);

        // printf("oi\n");
        Register reg_header;
        reg_header.removed = '0';
        reg_header.tam_reg = format_len(table->format, raw_data[i]) + register_header_size;
        next_byte_offset += reg_header.tam_reg;
        reg_header.data = table->data[i];
        reg_header.prox_reg = -1;

        table->register_headers[i] = reg_header;
    }

    table->num_fields = num_collumns;
    table->num_reg = num_rows;
    table->num_removed = 0;
    table->next_byte_offset = next_byte_offset;
    table->top = -1;
    table->has_index = false;

    return table;
}

Table *table_create_from_csv(CSV_handler *handler, char *format)
{
    return table_create(handler->data, format, handler->num_rows, handler->num_collumns);
}

void write_table_header(Table *table, char set)
{
    FILE *arquivo = table->f_pointer;

    int64_t ini_offset = ftell(arquivo);
    //printf("%d\n", ini_offset);
    fseek(arquivo, 0, SEEK_SET);

    fwrite(&set, sizeof(char), 1, arquivo);
    fwrite(&(table->top), sizeof(table->top), 1, arquivo);
    fwrite(&(table->next_byte_offset), sizeof(table->next_byte_offset), 1, arquivo);
    fwrite(&(table->num_reg), sizeof(table->num_reg), 1, arquivo);
    fwrite(&(table->num_removed), sizeof(table->num_removed), 1, arquivo);

    fseek(arquivo, ini_offset, SEEK_SET);
}

void write_register_header(FILE *arquivo, Register reghead)
{
    fwrite(&(reghead.removed), sizeof(reghead.removed), 1, arquivo);
    fwrite(&(reghead.tam_reg), sizeof(reghead.tam_reg), 1, arquivo);
    fwrite(&(reghead.prox_reg), sizeof(reghead.prox_reg), 1, arquivo);
}

void table_save(Table *table, char *path)
{
    FILE *arquivo;

    arquivo = fopen(path, "r_b");

    if (arquivo == NULL)
    {
        printf("Erro ao abrir o arquivo.");
        return;
    }

    table->f_pointer = arquivo;
    write_table_header(table, '0');

    for (int i = 0; i < table->data_size; i++)
    {
        Register reghead = table->register_headers[i];
        write_register_header(arquivo, reghead);
        fwrite((table->data[i]), sizeof(char), (reghead.tam_reg - register_header_size), arquivo);
    }

    write_table_header(table, '1');
    fseek(arquivo, table_header_size, SEEK_SET);

    fclose(arquivo);
}

void *temp_reg_data;
Register read_register(FILE *file)
{
    Register reg;

    reg.byte_offset = ftell(file);
    fread(&(reg.removed), sizeof(reg.removed), 1, file);
    fread(&(reg.tam_reg), sizeof(reg.tam_reg), 1, file);
    //printf("%d\n", reg.tam_reg);
    fread(&(reg.prox_reg), sizeof(reg.prox_reg), 1, file);

    int data_size = reg.tam_reg - register_header_size;

    free(temp_reg_data);
    temp_reg_data = malloc(data_size);
    fread(temp_reg_data, data_size, 1, file);
    reg.data = temp_reg_data;

    return reg;
}

Table *read_table_header(FILE *file, Table *table)
{
    fseek(file, 0, SEEK_SET);

    table->pos_reg = -1;
    char status = '1';

    fread(&status, sizeof(char), 1, file);
    if (status == '0')
        return NULL;

    fread(&(table->top), sizeof(table->top), 1, file);
    fread(&(table->next_byte_offset), sizeof(table->next_byte_offset), 1, file);
    fread(&(table->num_reg), sizeof(table->num_reg), 1, file);
    fread(&(table->num_removed), sizeof(table->num_removed), 1, file);

    return table;
}

Table *table_access(char *path, char *format)
{
    FILE *file = fopen(path, "r+b");

    if (file == NULL)
    {
        return NULL;
    }

    Table *table = malloc(sizeof(Table));
    table->f_pointer = file;
    table->format = decode_format(format);
    // printf("%s", table->format);
    if (!read_table_header(file, table))
    {
        return NULL;
    }

    table_reset_register_pointer(table);
    table->has_index = false;
    table->index.key = NULL;
    table->index.byte_offset = NULL;
    return table;
}

bool table_move_to_next_register(Table *table)
{
    if (table->pos_reg >= table->num_reg - 1)
        return false;

    table->current_register = read_register(table->f_pointer);
    if (table->current_register.removed == '1')
    {
        return table_move_to_next_register(table);
    }
    table->pos_reg += 1;

    return true;
}

bool table_move_to_previous_register(Table *table)
{
    if (table->pos_reg <= 0)
        return false;

    fseek(table->f_pointer, - table->current_register.tam_reg, SEEK_CUR);
    table->current_register = read_register(table->f_pointer);

    if (table->current_register.removed == '1')
    {
        return table_move_to_previous_register(table);
    }
    table->pos_reg -= 1;

    return true;
}

void table_reset_register_pointer(Table *table)
{
    fseek(table->f_pointer, table_header_size, SEEK_SET);
    table->pos_reg = -1;
}

char *decode_format(char *format)
{
    int tamanho = strlen(format);
    char *tipos = (char *)malloc(tamanho * sizeof(char));
    int contador = 0;

    for (int i = 0; i < tamanho; i++)
    {
        if (format[i] == '%' && i < tamanho - 1)
        {
            tipos[contador] = format[i + 1];
            contador++;
        }
    }

    tipos[contador] = '\0';
    return tipos;
}

void *format_data(char *format, char **data)
{
    void *formated_data;
    // printf("%d\n", format_len(format, data));

    formated_data = (void *)calloc(format_len(format, data) + 1, sizeof(void *));
    char *aux_ptr = format;
    int counter = 0;
    int size_ptr = 0;

    while (*aux_ptr != '\0')
    {
        if (*aux_ptr == 'd')
        {
            int value = atoi(data[counter]);
            memcpy(formated_data + size_ptr, (void *)(&value), 4);

            counter += 1;
            size_ptr += 4;
        }
        else if (*aux_ptr == 's')
        {
            if (strcmp(data[counter], "$") == 0)
            {
                counter += 1;
                size_ptr += 4;
                aux_ptr++;
                continue;
            }

            int str_size = strlen(data[counter]);
            memcpy(formated_data + size_ptr, &str_size, 4);
            memcpy(formated_data + size_ptr + 4, data[counter], str_size);

            counter += 1;
            size_ptr += str_size + 4;
        }
        aux_ptr++;
    }

    return formated_data;
}

int format_len(char *format, char **data)
{
    int size = 0;
    char *aux_ptr = format;
    int counter = 0;

    while (*aux_ptr != '\0')
    {
        // printf("%c\n", *aux_ptr);
        if (*aux_ptr == 'd')
        {
            size += 4;
            counter += 1;
        }
        else if (*aux_ptr == 's')
        {
            if (strcmp(data[counter], "$") == 0)
            {
                counter += 1;
                size += 4;
                aux_ptr++;
                continue;
            }
            // printf("%s\n", data[counter]);
            // printf("%d\n", strlen(data[counter]));
            size += strlen(data[counter]) + 4;
            counter += 1;
        }

        aux_ptr++;
    }

    return size;
}

void *table_get_current_register_data_by_index(Table *table, int index)
{
    int size = 0;
    char *aux_ptr = table->format;
    // printf("%s\n", table->format);
    int counter = 0;

    void *ret_val;

    while (*aux_ptr != '\0')
    {
        // printf("%c\n", *aux_ptr);
        if (*aux_ptr == 'd')
        {
            if (counter == index)
            {
                void *data = table->current_register.data + size;
                ret_val = calloc(4, 1);
                memcpy(ret_val, data, 4);
                return ret_val;
            }

            size += 4;
        }
        else if (*aux_ptr == 's')
        {

            void *data = table->current_register.data + size;
            // printf("%d\n", *((int32_t*)(data)));

            if (counter == index)
            {
                ret_val = calloc(*((int32_t *)(data)) + 1, 1);
                memcpy(ret_val, data + 4, *((int32_t *)(data)));
                return ret_val;
            }

            size += *((int32_t *)data) + 4;
        }
        counter++;
        aux_ptr++;
    }

    return NULL;
}

int search_state = 0;
bool table_search_for_matches(Table *table, void **data, int *indexes, int num_parameters)
{
    if (search_state == 0)
    {
        table_reset_register_pointer(table);

        for (int i = 0; i < num_parameters; i++)
            if (indexes[i] == 0 && table->has_index)
            {
                bool sr = table_search_using_index(table, data[i]);
                if (sr)
                    search_state = 1;
                
                return sr;
            }

        search_state = 2;
    }
    else if (search_state == 1)
    {
        //printf("passou aqui?\n");
        search_state = 0;
        return false;
    }
    while (table_move_to_next_register(table))
    {
        bool match = true;

        for (int i = 0; i < num_parameters; i++)
        {
            void *v = table_get_current_register_data_by_index(table, indexes[i]);
            void *value = v;
            void *data_temp = data[i];

            if (table->format[indexes[i]] == 's')
            {
                //printf("a:%s| b:%s|\n", value, data_temp);
                if (strcmp(data_temp, value) != 0)
                {
                    match = false;
                    break;
                }
            }
            else if (table->format[indexes[i]] == 'd')
            {
                //printf("a:%d b:%d\n", *(int32_t*)value, *(int32_t*)data[i]);
                if (*(int32_t *)data[i] != *(int32_t *)value)
                {
                    match = false;
                    break;
                }
            }

            free(v);
        }

        if (match)
            return true;
    }

    search_state = 0;
    //printf("passou aqyu efgbaseyuz\n");

    return false;
}

void free_void_list(void ***collumn, int items)
{
    void **col = *(collumn);
    for (int j = 0; j < items; j++)
    {
        free(col[j]);
    }
    free(col);
    *(collumn) = NULL;
}

void table_free(Table **tab)
{
    if ((*tab)->f_pointer)
        fclose((*tab)->f_pointer);
    if ((*tab)->register_headers)
    {
        for (int i = 0; i < (*tab)->num_reg; i++)
        {
            free((*tab)->register_headers);
            (*tab)->register_headers = NULL;
        }
    }
    if ((*tab)->data)
        free_void_list(&((*tab)->data), (*tab)->data_size);
    free(*tab);
    (*tab) = NULL;
}

bool table_create_index(Table *table, char *path, int key_row, int key_size)
{
    FILE *arquivo;
    arquivo = fopen(path, "wb");

    if (arquivo == NULL)
    {
        printf("Erro ao abrir o arquivo.");
        return false;
    }

    table_reset_register_pointer(table);
    fwrite("0", sizeof(char), 1, arquivo);

    while (table_move_to_next_register(table))
    {
        // printf("oi\n");
        // printf("%d\n", *((int*)table->current_register.data));
        fwrite(table->current_register.data + key_row * sizeof(char), sizeof(char), key_size, arquivo);
        fwrite(&(table->current_register.byte_offset), sizeof(table->current_register.byte_offset), 1, arquivo);
    }

    table_reset_register_pointer(table);

    fseek(arquivo, 0, SEEK_SET);
    fwrite("1", sizeof(char), 1, arquivo);

    fclose(arquivo);
    
    table->index.key_size = key_size;
    table->index.key_row = key_row;

    return true;
}

bool table_load_index(Table *table, char *path)
{
    FILE *arquivo;
    arquivo = fopen(path, "r+b");

    if (arquivo == NULL)
    {
        printf("Erro ao abrir o arquivo.");
        return false;
    }

    fseek(arquivo, 0, SEEK_SET);

    fwrite("0", sizeof(char), 1, arquivo);
    fseek(arquivo, 1, SEEK_SET);


    if (table->index.key != NULL)
    {
        free(table->index.key);
        table->index.key = NULL;
    }

    if (table->index.byte_offset != NULL)
    {
        free(table->index.byte_offset);
        table->index.byte_offset = NULL;
    }

    table->index.key = malloc(sizeof(void*) * table->num_reg);
    for (int i = 0; i < table->num_reg; i++)
    {
        //printf("%d\n", table->index.key_size);
        table->index.key[i] = malloc(table->index.key_size);
    }

    table->index.byte_offset = malloc(sizeof(int64_t) * table->num_reg);

    for (int i = 0; i < table->num_reg; i++)
    {
        //printf("%d\n", i);
        fread((table->index.key[i]), sizeof(char), table->index.key_size, arquivo);
        //printf("%d\n", *((int32_t *)(table->index.key[i])));
        fread(&(table->index.byte_offset[i]), sizeof(char), 8, arquivo);
    }

    fseek(arquivo, 0, SEEK_SET);
    fwrite("1", sizeof(char), 1, arquivo);
    fclose(arquivo);

    table->has_index = true;
    return true;
}

bool rewrite_current_register(Table* table)
{
    fseek(table->f_pointer, - table->current_register.tam_reg, SEEK_CUR);
    write_register_header(table->f_pointer, table->current_register);
    fseek(table->f_pointer, -register_header_size, SEEK_CUR);

    return true;
}


bool table_delete_current_register(Table *table)
{

    write_table_header(table, '0');
    table->current_register.removed = '1';

    int64_t aux = table->top;
    table->top = table->current_register.byte_offset;
    table->current_register.prox_reg = aux;

    table->num_removed += 1;
    table->num_reg -= 1;
    table->pos_reg -= 1;

    rewrite_current_register(table);
    write_table_header(table, '1');

    return true;
}


bool table_search_using_index(Table *table, void *key)
{
    for (int i = 0; i < table->num_reg; i++)
    {
        //printf("oi\n");
        //printf("%d %d\n", *((int32_t*)(table->index.key[i])), *((int32_t*)key));
        if (*((int32_t*)(table->index.key[i])) == *((int32_t*)key))
        {
            fseek(table->f_pointer, table->index.byte_offset[i], SEEK_SET);
            return table_move_to_next_register(table);
        }
    }

    return false;
}


int64_t find_best_fit(Table* table, int tam)
{
    int64_t cur_offset = ftell(table->f_pointer);

    int64_t ret_offset = -1;
    int64_t aux_offset = table->top;

    int best_fit = -1;
    int aux_fit;

    while (aux_offset != -1)
    {
        fseek(table->f_pointer, aux_offset + 1, SEEK_SET);
        fread(&aux_fit, sizeof(int), 1, table->f_pointer);
        fread(&aux_offset, sizeof(int64_t), 1, table->f_pointer);

        aux_fit -= tam;

        if ((aux_fit >= 0 && aux_fit < best_fit) || best_fit < 0)
        {
            best_fit = aux_fit;
            ret_offset = aux_offset;
        }
    }

    if (best_fit < 0)
    {
        fseek(table->f_pointer, 0, SEEK_END);
        ret_offset = ftell(table->f_pointer);
    }
    
    fseek(table->f_pointer, cur_offset, SEEK_SET);
    return ret_offset;
}

bool table_insert_new_register(Table* table, void *data, int data_size)
{

    int64_t cur_offset = ftell(table->f_pointer);

    int64_t best_fit = find_best_fit(table, data_size + register_header_size);

    int tam_reg;

    fseek(table->f_pointer, best_fit + 1, SEEK_SET);
    fread(&tam_reg, sizeof(int), 1, table->f_pointer);
    fseek(table->f_pointer, best_fit + 1, SEEK_SET);

    Register new_register;
    new_register.data = data;
    new_register.prox_reg = -1;
    new_register.removed = '0';
    new_register.tam_reg = tam_reg;
    
    write_register_header(table->f_pointer, new_register);
    fwrite(data, sizeof(char), data_size, table->f_pointer);
    
    int remainig_space = tam_reg - data_size;

    while (remainig_space > 0)
    {
        fputc('$', table->f_pointer);
        remainig_space -= 1;
    }
    
    return true;
}