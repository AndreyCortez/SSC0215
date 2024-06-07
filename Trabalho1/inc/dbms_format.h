#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

void *format_data(char *format, char **data);
int format_len(char *format, char **data);
char *decode_format(char *format);
void *get_data_in_collumn(void *data, char *format, int collumn);