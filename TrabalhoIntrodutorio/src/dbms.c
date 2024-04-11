#include "dbms.h"

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
