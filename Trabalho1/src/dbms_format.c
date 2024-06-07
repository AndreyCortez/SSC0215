#include "dbms_format.h"

void *format_data(char *format, char **data)
{
    void *formated_data;

    formated_data = (void *)calloc(format_len(format, data) + 1, sizeof(void *));
    char *aux_ptr = format;
    int counter = 0;
    int size_ptr = 0;

    while (*aux_ptr != '\0')
    {
        if (*aux_ptr == 'd')
        {
            if (strcmp(data[counter], "$") == 0 || strcmp(data[counter], "NULO") == 0)
            {
                int null_value = -1;
                memcpy((char*)formated_data + size_ptr, (void *)(&null_value), 4);
            }
            else
            {
                int value = atoi(data[counter]);
                memcpy((char*)formated_data + size_ptr, (void *)(&value), 4);
            }

            counter += 1;
            size_ptr += 4;
        }
        else if (*aux_ptr == 's')
        {
            int str_size = 0;

            if (strcmp(data[counter], "$") != 0 && strcmp(data[counter], "NULO") != 0)
            {
                str_size = strlen(data[counter]);
                memcpy((char*)formated_data + size_ptr, &str_size, 4);
                memcpy((char*)formated_data + size_ptr + 4, data[counter], str_size);
            }

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
        if (*aux_ptr == 'd')
        {
            size += 4;
            counter += 1;
        }
        else if (*aux_ptr == 's')
        {
            if (strcmp(data[counter], "$") == 0 || strcmp(data[counter], "NULO") == 0)
            {
                counter += 1;
                size += 4;
                aux_ptr++;
                continue;
            }

            size += strlen(data[counter]) + 4;
            counter += 1;
        }

        aux_ptr++;
    }

    return size;
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

void *get_data_in_collumn(void *data, char *format, int index)
{
    int size = 0;
    char *aux_ptr = format;

    int counter = 0;
    void *ret_val;

    while (*aux_ptr != '\0')
    {
        if (*aux_ptr == 'd')
        {
            if (counter == index)
            {
                void *_d = (char*)data + size;
                //printf("%d %d %d\n", *((int32_t *)_d), *((int32_t *)data), size);
                ret_val = calloc(4, 1);
                memcpy(ret_val, _d, 4);
                return ret_val;
            }

            size += 4;
        }
        else if (*aux_ptr == 's')
        {

            void *_d = (char*)data + size;

            if (counter == index)
            {
                ret_val = calloc(*((int32_t *)(_d)) + 1, 1);
                memcpy(ret_val, (char*)_d + 4, *((int32_t *)(_d)));
                return ret_val;
            }

            size += *((int32_t *)_d) + 4;
        }
        counter++;
        aux_ptr++;
    }

    return NULL;
}