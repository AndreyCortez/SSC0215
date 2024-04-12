#include "dbms.h"

Table *table_create(char ***raw_data, char *format, int num_rows, int num_collumns)
{
    Table *table = malloc(sizeof(table));

    table->data = malloc(sizeof(void *) * num_rows);
    table->register_headers = malloc(sizeof(RegHeader) * num_rows);

    uint64_t next_byte_offset = 0;
    const uint32_t len_reg_header = 24;

    for (int i = 0; i < num_rows; i++)
    {

        table->data[i] = format_data(format, raw_data[i]);

        RegHeader reg_header;
        reg_header.removed = 0;
        reg_header.tam_reg = format_len(format, raw_data[i]) + len_reg_header;
        next_byte_offset += reg_header.tam_reg;
        reg_header.data = table->data[i];
        reg_header.prox_reg = next_byte_offset;

        table->register_headers[i] = reg_header;
    }

    TableHandler *handler = malloc(sizeof(TableHandler));

    handler->num_fields = num_collumns;
    handler->num_reg = num_rows;
    handler->num_removed = 0;
    handler->next_byte_offset = 0;
    handler->top = -1;

    table->handler = handler;

    return table;
}

Table *table_create_from_csv(CSV_handler *handler, char *format)
{
    return table_create(handler->data, format, handler->num_rows, handler->num_collumns);
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
                memcpy(aux_ptr, &value_d, sizeof(int));
                aux_ptr += sizeof(int);
                break;

            case 's':
                size = strlen(data[counter]);

                memcpy(aux_ptr, &size, sizeof(int));
                aux_ptr += sizeof(int);

                memcpy(aux_ptr, data[counter], size);
                aux_ptr += size;
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
            putchar(*ptr);
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
            //printf("oi\n");
            switch (*(ptr + 1))
            {
            case 'd':

                size += sizeof(int);
                break;

            case 's':
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
            putchar(*ptr);
            ptr++;
        }
    }

    return size;
}
