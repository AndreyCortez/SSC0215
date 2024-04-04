#include <csv.h>

CSV_handler *csv_parse(FILE *file, bool skip_first_line)
{
    char line[MAX_LINE_LENGTH];
    char *token;

    int data_size = skip_first_line ? -1 : 0;
    int line_size = 0;

    while (fgets(line, sizeof(line), file))
    {
        int counter = 0;
        token = strtok(line, ",");

        while (token != NULL)
        {

            counter += 1;

            char *newline = strchr(token, '\n');

            if (newline)
            {
                *newline = '\0';
                line_size = counter;
                counter = 0;
                data_size += 1;
            }

            token = strtok(NULL, ",");
        }
    }

    printf("O arquivo possui %d linhas e %d colunas\n", data_size, line_size);

    CSV_handler *csv_handler;
    csv_handler = malloc(sizeof(CSV_handler));

    csv_handler->data = (char ***)malloc(sizeof(char **) * data_size);

    for (int i = 0; i < data_size; i++)
    {
        csv_handler->data[i] = (char **)malloc(sizeof(char *) * line_size);
    }

    csv_handler->num_rows = data_size;
    csv_handler->num_collumns = line_size;

    fseek(file, 0, SEEK_SET);

    int current_row = 0;
    int current_collum = 0;

    char c;

    while ((c = fgetc(file)) != EOF)
    {
        if (c == '\n')
            break;
    }

    while (fgets(line, sizeof(line), file))
    {

        token = strtok(line, ",");

        while (token != NULL)
        {
            char *field_data;
            // NOTE: malloc dentro de um loop pode ser que isso deixe o programa lento
            field_data = malloc((strlen(token) + 1) * sizeof(char));
            strcpy(field_data, token);

            csv_handler->data[current_row][current_collum] = field_data;
            // printf("%s %d %d\n", csv_handler->data[current_row][current_collum], current_row, current_collum);

            current_collum += 1;

            char *newline = strchr(token, '\n');

            if (newline)
            {
                *newline = '\0';
                current_row += 1;
                current_collum = 0;
            }

            token = strtok(NULL, ",");
        }
    }

    return csv_handler;
}

void csv_print_head(CSV_handler *handler)
{
    if (handler == NULL)
        return;

    int num_lines = (5 < (handler->num_rows)) ? 5 : handler->num_rows;

    for (int i = 0; i < num_lines; i++)
    {
        for (int j = 0; j < handler->num_collumns; j++)
        {
            printf("%s, ", handler->data[i][j]);
        }
        printf("\n");
    }
}



