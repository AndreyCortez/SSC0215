#ifndef DBMS
#define DBMS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct
{
    void **data;
    int data_size;
    int fields;
    char* format;
} table;

typedef struct
{
    /* data */
}table_header;

typedef struct
{
    /* data */
}table_item;



table *table_create(void **raw_data, char* format);
void *add_raw_item(table* table, void *raw_item);

void *format_data(const char *format, void* data) {
    const char *ptr = format;
    void *formated_data;

    formated_data = (void*)malloc(sizeof(void*) * format_len(format));
    void *aux_ptr = formated_data;

    int counter = 0;

    while (*ptr != '\0') {
        if (*ptr == '%') {
            switch (*(ptr + 1)) {
                case 'd': {
                    
                }
                case 's': {
                   
                }
                case 'f': {
                    
                }
                default:
                    return NULL;
            }
            counter += sizeof(void*);
            ptr += 2;
        } else {
            putchar(*ptr);
            ptr++;
        }
    }
}

int format_len(const char *format)
{
    const char *ptr = format;
    void *formated_data;
    int size = 0;

    while (*ptr != '\0') {
        if (*ptr == '%') {
            switch (*(ptr + 1)) {
                case 'd': {
                    size += sizeof(int);
                }
                case 's': {
                   size += sizeof(char *);
                }
                case 'f': {
                    size += sizeof(float);
                }
                default:
                    return 0;
            }
            ptr += 2;
        } else {
            putchar(*ptr);
            ptr++;
        }
    }

    return size;
}

#endif