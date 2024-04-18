#include "dbms.h"

Table *table_create(char ***raw_data, char *format, int num_rows, int num_collumns)
{
    Table *table;
    table = (Table *)malloc(sizeof(Table));

    table->data = (void **)malloc(sizeof(void *) * num_rows);
    table->data_size = num_rows;
    table->register_headers = (RegHeader *)malloc(sizeof(RegHeader) * num_rows);

    uint64_t next_byte_offset = 25;
    const uint32_t len_reg_header = 13;

    for (int i = 0; i < num_rows; i++)
    {
        
        table->data[i] = format_data(format, raw_data[i]);

        RegHeader reg_header;
        reg_header.removed = '0';
        reg_header.tam_reg = format_len(format, raw_data[i]) + len_reg_header;
        next_byte_offset += reg_header.tam_reg;
        reg_header.data = table->data[i];
        reg_header.prox_reg = -1;

        table->register_headers[i] = reg_header;
    }

    TableHeader *handler = malloc(sizeof(TableHeader));

    handler->status = '1';
    handler->num_fields = num_collumns;
    handler->num_reg = num_rows;
    handler->num_removed = 0;
    handler->next_byte_offset = next_byte_offset;
    handler->top = -1;

    table->handler = handler;

    return table;
}

Table *table_create_from_csv(CSV_handler *handler, char *format)
{
    return table_create(handler->data, format, handler->num_rows, handler->num_collumns);
}

void write_table_header(FILE* arquivo, TableHeader* tabhead)
{
    fwrite(&(tabhead->status), sizeof(tabhead->status), 1, arquivo);
    fwrite(&(tabhead->top), sizeof(tabhead->top), 1, arquivo);
    fwrite(&(tabhead->next_byte_offset), sizeof(tabhead->next_byte_offset), 1, arquivo);
    fwrite(&(tabhead->num_reg), sizeof(tabhead->num_reg), 1, arquivo);
    fwrite(&(tabhead->num_removed), sizeof(tabhead->num_removed), 1, arquivo);
}

void write_register_header(FILE* arquivo, RegHeader reghead)
{
    fwrite(&(reghead.removed), sizeof(reghead.removed), 1, arquivo);
    fwrite(&(reghead.tam_reg), sizeof(reghead.tam_reg), 1, arquivo);
    fwrite(&(reghead.prox_reg), sizeof(reghead.prox_reg), 1, arquivo);
}

void table_save(Table *table, char* path)
{
    FILE *arquivo;

    arquivo = fopen(path, "wb");

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo.");
        return;
    }

    // Escreve o cabeçalho do arquivo

    TableHeader *tabhead = table->handler;

    write_table_header(arquivo, tabhead);
    
    
    for (int i = 0; i < table->data_size; i++)
    {
        RegHeader reghead = table->register_headers[i];
        write_register_header(arquivo, reghead);
        fwrite((table->data[i]), sizeof(char), (reghead.tam_reg - 13), arquivo);
    }

    fclose(arquivo);
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

            //printf("%s\n", data[counter]);

            switch (*(ptr + 1))
            {
            case 'd':

                value_d = atoi(data[counter]);
                if (value_d == 0 && strcmp(data[counter], "0") != 0)
                {
                    // printf("inteiro faltando\n");
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
            //putchar(*ptr);
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

    // printf("%s\n", format);

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
