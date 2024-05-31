#include "dbms.h"

Table *table_create(char ***raw_data, char *format, int num_rows, int num_collumns)
{
    Table *table;
    table = (Table *)malloc(sizeof(Table));

    table->data = (void **)malloc(sizeof(void *) * num_rows);
    table->data_size = num_rows;
    table->register_headers = (Register *)malloc(sizeof(Register) * num_rows);

    uint64_t next_byte_offset = table_header_size;

    for (int i = 0; i < num_rows; i++)
    {

        table->data[i] = format_data(format, raw_data[i]);

        Register reg_header;
        reg_header.removed = '0';
        reg_header.tam_reg = format_len(format, raw_data[i]) + register_header_size;
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

    return table;
}

Table *table_create_from_csv(CSV_handler *handler, char *format)
{
    return table_create(handler->data, format, handler->num_rows, handler->num_collumns);
}

void write_table_header(FILE *arquivo, Table *tabhead)
{
    fwrite(&(tabhead->top), sizeof(tabhead->top), 1, arquivo);
    fwrite(&(tabhead->next_byte_offset), sizeof(tabhead->next_byte_offset), 1, arquivo);
    fwrite(&(tabhead->num_reg), sizeof(tabhead->num_reg), 1, arquivo);
    fwrite(&(tabhead->num_removed), sizeof(tabhead->num_removed), 1, arquivo);
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

    arquivo = fopen(path, "wb");

    if (arquivo == NULL)
    {
        printf("Erro ao abrir o arquivo.");
        return;
    }

    fwrite("0", sizeof(char), 1, arquivo);
    write_table_header(arquivo, table);

    for (int i = 0; i < table->data_size; i++)
    {
        Register reghead = table->register_headers[i];
        write_register_header(arquivo, reghead);
        fwrite((table->data[i]), sizeof(char), (reghead.tam_reg - register_header_size), arquivo);
    }

    fseek(arquivo, 0, SEEK_SET);
    fwrite("1", sizeof(char), 1, arquivo);
    fclose(arquivo);
}

void *temp_reg_data;
Register read_register(FILE *file)
{
    Register reg;

    reg.byte_offset = ftell(file);
    fread(&(reg.removed), sizeof(reg.removed), 1, file);
    fread(&(reg.tam_reg), sizeof(reg.tam_reg), 1, file);
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

Table *table_access(char *path, char* format)
{
    FILE *file = fopen(path, "rb");

    if (file == NULL)
    {
        return NULL;
    }

    Table *table = malloc(sizeof(Table));
    table->f_pointer = file;
    table->format = format;
    if (!read_table_header(file, table))
    {
        return NULL;
    }

    table_reset_register_pointer(table);
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

void table_reset_register_pointer(Table *table)
{
    fseek(table->f_pointer, table_header_size, SEEK_SET);
    table->pos_reg = -1;
}

void *format_data(const char *format, char **data)
{
    const char *ptr = format;
    void *formated_data;

    formated_data = (void *)malloc(sizeof(void *) * format_len(format, data));
    void *aux_ptr = formated_data;

    int counter = 0;

    while (*ptr != '\0')
    {
        if (*ptr == '%')
        {

            int value_d;
            int size;
            float value_f;

            switch (*(ptr + 1))
            {
            case 'd':

                value_d = atoi(data[counter]);
                if (value_d == 0 && strcmp(data[counter], "0") != 0)
                {
                    value_d = -1;
                }
                memcpy(aux_ptr, &value_d, sizeof(int));
                aux_ptr += sizeof(int);
                break;

            case 's':
                if (strcmp(data[counter], "$") == 0)
                {
                    size = 0;
                    memcpy(aux_ptr, &size, sizeof(int));
                }
                else
                {
                    size = strlen(data[counter]);

                    memcpy(aux_ptr, &size, sizeof(int));
                    aux_ptr += sizeof(int);

                    memcpy(aux_ptr, data[counter], size);
                    aux_ptr += size;
                }
                break;

            case 'f':

                value_f = atof(data[counter]);
                memcpy(aux_ptr, &value_f, sizeof(float));
                aux_ptr += sizeof(float);
                break;

            default:

                return NULL;
                break;
            }
            ptr += 2;
            counter += 1;
        }
        else
        {
            ptr++;
        }
    }

    return formated_data;
}

int format_len(const char *format, char **data)
{
    const char *ptr = format;
    int size = 0;
    int counter = 0;

    while (*ptr != '\0')
    {
        if (*ptr == '%')
        {
            switch (*(ptr + 1))
            {
            case 'd':

                size += sizeof(int);
                break;

            case 's':
                if (strcmp(data[counter], "$") == 0)
                    size += sizeof(int);
                else
                    size += strlen(data[counter]) + sizeof(int);
                break;

            case 'f':

                size += sizeof(float);
                break;

            default:
                return 0;
                break;
            }
            ptr += 2;
            counter += 1;
        }
        else
        {
            ptr++;
        }
    }

    return size;
}

void *table_get_current_register_data_by_index(Table * table, char* format, int index)
{
    const char *ptr = format;
    void *last_data_ptr = table->current_register.data;
    void *data_ptr = last_data_ptr;
    int data_size = 0;
    int counter = 0;

    while (*ptr != '\0')
    {
        if (*ptr == '%')
        {
            switch (*(ptr + 1))
            {
            case 'd':
                data_size = sizeof(int32_t);
                last_data_ptr = data_ptr;
                data_ptr += data_size;
                break;

            case 's':
                data_size = *((int32_t *)(data_ptr));
                last_data_ptr = data_ptr;
                data_ptr += data_size + sizeof(int32_t);
                break;

            case 'f':
                last_data_ptr = data_ptr;
                data_size = sizeof(float);
                data_ptr += sizeof(float);
                break;

            default:
                return NULL;
                break;
            }

            if (counter == index)
            {
                data_ptr = calloc((data_size + 1), sizeof(char));
                memccpy(data_ptr, last_data_ptr, sizeof(char), data_size);
                return data_ptr;
            }

            ptr += 2;
            counter += 1;
        }
        else
        {
            ptr++;
        }
    }

    return NULL;
}


bool search_state = 0;
bool table_search_for_matches(Table *table, void** data, int* indexes, int num_parameters)
{
    if (search_state == 0)
    {
        for (int i = 0; i < num_parameters; i++)
            if (indexes[i] == 0 & table->index != NULL)
            {
                bool sr = table_search_using_index(table, data[i]);
                if (sr)
                    search_state = 1;
                return sr;
            }

        search_state = 2;
        table_reset_register_pointer(table);
    }
    else if (search_state == 1)
    {
        return false;
    }
    while (table_move_to_next_register(table))
    {
        bool match = true;

        for (int i = 0; i < num_parameters; i++)
        {
            void *v =  table_get_current_register_data_by_index(table, table->format, i);
            void *value = v;
            void *data_temp = data[i];
            
            while (value != '\0')
            {
                if (data_temp != value)
                {
                    match = false;
                    break;
                }

                value += 1;
            }

            free(v);
        }

        if (match)
            return true; 
    }

    search_state = 0;
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

bool table_create_index(Table *table, char* path, int key_row, int key_size)
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
        //printf("%d\n", *((int*)table->current_register.data));
        fwrite(table->current_register.data + key_row * sizeof(char), sizeof(char), key_size, arquivo);
        fwrite(&(table->current_register.byte_offset), sizeof(table->current_register.byte_offset), 1, arquivo);
    }

    fseek(arquivo, 0, SEEK_SET);
    fwrite("1", sizeof(char), 1, arquivo);

    fclose(arquivo);
    return true;
}

bool table_load_index(Table *table, char*  path)
{
    FILE *arquivo;
    arquivo = fopen(path, "wb");

    if (arquivo == NULL)
    {
        printf("Erro ao abrir o arquivo.");
        return false;
    }

    fwrite("0", sizeof(char), 1, arquivo);

    for (int i = 0; i < table->num_reg; i++)
    {
        fread((table->index.key[i]) , sizeof(char), table->index.key_size, arquivo);
        fread(&(table->index.byte_offset[i]), sizeof(char), table->index.key_size, arquivo);
    }

    fseek(arquivo, 0, SEEK_SET);
    fwrite("1", sizeof(char), 1, arquivo);
    fclose(arquivo);

    return true;
}


bool table_search_using_index(Table *tab, void* key)
{
    return false;
}
// bool table_delete_using_index(Table *tab, void* key);