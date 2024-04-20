#include <csv.h>

char *paxtok(char *str, char *seps)
{
    static char *tpos, *tkn, *pos = NULL;
    static char savech;

    if (str != NULL)
    {
        pos = str;
        savech = 'x';
    }
    else
    {
        if (pos == NULL)
            return NULL;
        while (*pos != '\0')
            pos++;
        *pos++ = savech;
    }

    if (savech == '\0')
        return NULL;

    tpos = pos;
    while (*tpos != '\0')
    {
        tkn = strchr(seps, *tpos);
        if (tkn != NULL)
            break;
        tpos++;
    }

    savech = *tpos;
    *tpos = '\0';

    return pos;
}

CSV_handler *csv_parse(FILE *file, bool has_header)
{
    char line[MAX_LINE_LENGTH];
    char *token;

    int data_size = 0;
    int line_size = 0;

    int counter = 0;

    while (fgets(line, sizeof(line), file))
    {
        token = paxtok(line, ",");

        while (token != NULL)
        {
            char *newline = strchr(token, '\n');

            if (newline)
            {
                *newline = '\0';
                data_size += 1;
                line_size = counter;
                counter = 0;
            }

            token = paxtok(NULL, ",");
            if (token != NULL)
                counter += 1;
        }
    }

    if (counter == 0)
        data_size -= 1;

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

    if (has_header)
    {
        csv_handler->header = (char **)malloc(line_size * sizeof(char *));
        bool header_complete = false;

        while (fgets(line, sizeof(line), file) && !header_complete)
        {
            int counter = 0;
            token = paxtok(line, ",");

            while (token != NULL && !header_complete)
            {
                char *field_data;

                field_data = malloc((strlen(token) + 1) * sizeof(char));
                strcpy(field_data, token);

                csv_handler->header[counter] = field_data;

                counter += 1;

                char *newline = strchr(token, '\n');

                if (newline)
                {
                    free(field_data);
                    fseek(file, 0, SEEK_SET);
                    header_complete = true;
                }

                token = paxtok(NULL, ",");
            }
        }
    }

    int current_row = 0;
    int current_collum = 0;

    char c;

    fseek(file, 0, SEEK_SET);

    while ((c = fgetc(file)) != EOF && has_header)
    {
        if (c == '\n')
            break;
    }

    while (fgets(line, sizeof(line), file))
    {

        token = paxtok(line, ",");
        char *prev_token_end = line;
        while (token != NULL)
        {
            char *field_data;
            char *newline = strchr(token, '\n');

            if (newline)
            {
                *newline = '\0';
            }

            if (prev_token_end == token && token != newline && strcmp(token, "") != 0)
            {
                field_data = malloc((strlen(token) + 1) * sizeof(char));
                strcpy(field_data, token);
            }
            else
            {
                field_data = "$";
            }

            csv_handler->data[current_row][current_collum] = field_data;

            current_collum += 1;

            if (newline)
            {
                free(field_data);
                current_row += 1;
                current_collum = 0;
            }

            prev_token_end = token + strlen(token) + 1;
            token = paxtok(NULL, ",");
        }
    }
    return csv_handler;
}

void csv_print_head(CSV_handler *handler)
{
    if (handler == NULL)
        return;

    int num_lines = (10 < (handler->num_rows)) ? 10 : handler->num_rows;

    for (int i = 0; i < num_lines; i++)
    {
        for (int j = 0; j <= handler->num_collumns; j++)
        {
            printf("%s, ", handler->data[i][j]);
        }
        printf("\n");
    }
}

int csv_find_collumn(CSV_handler *handler, char *header)
{
    for (int i = 0; i < handler->num_collumns; i++)
    {
        if (strcmp(header, handler->header[i]) == 0)
        {
            return i;
        }
    }

    return -1;
}

char **csv_retrieve_collumn(CSV_handler *handler, char *collumn)
{
    int collumn_num = csv_find_collumn(handler, collumn);

    if (collumn_num != -1)
    {
        char **result = (char **)malloc(handler->num_rows * sizeof(char *));
        for (int i = 0; i < handler->num_rows; i++)
        {
            char *res_str = handler->data[i][collumn_num];
            result[i] = malloc((strlen(res_str) + 1) * sizeof(char));
            strcpy(result[i], res_str);
        }
        return result;
    }
    else
    {
        return NULL;
    }
}

CSV_handler *csv_retrieve_collumns(CSV_handler *handler, char **collumns, int qtd_collumns)
{
    for (int i = 0; i < qtd_collumns; i++)
    {
        if (csv_find_collumn(handler, collumns[i]) == -1)
            return NULL;
    }

    CSV_handler *ret_handler;
    ret_handler = malloc(sizeof(CSV_handler));

    ret_handler->num_rows = handler->num_rows;
    ret_handler->num_collumns = qtd_collumns;
    ret_handler->data = malloc(ret_handler->num_rows * sizeof(char **));
    ret_handler->header = malloc((qtd_collumns * sizeof(char *)));

    for (int i = 0; i < ret_handler->num_rows; i++)
    {
        ret_handler->data[i] = malloc(ret_handler->num_collumns * sizeof(char *));
    }

    for (int i = 0; i < qtd_collumns; i++)
    {
        ret_handler->header[i] = malloc((strlen(collumns[i]) + 1) * sizeof(char));
        strcpy(ret_handler->header[i], collumns[i]);

        char **aux_data = csv_retrieve_collumn(handler, collumns[i]);

        for (int j = 0; j < ret_handler->num_rows; j++)
            ret_handler->data[j][i] = aux_data[j];

        free(aux_data);
    }

    return ret_handler;
}

void free_matrix(char ****matrix, int rows, int collumns)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < collumns; j++)
        {
            free((*matrix)[i][j]);
            (*matrix)[i][j] = NULL;
        }
        free((*matrix)[i]);
        (*matrix)[i] = NULL;
    }
    free((*matrix));
    *matrix = NULL;
}

void free_char_list(char ***collumn, int items)
{
    for (int j = 0; j < items; j++)
    {
        free((*collumn)[j]);
    }
    free((*collumn));
    *collumn = NULL;
}

void csv_free_handle(CSV_handler **handler)
{

    if ((*handler)->data != NULL)
        free_matrix(&((*handler)->data), (*handler)->num_rows, (*handler)->num_collumns);
    if ((*handler)->header != NULL)
        free_char_list(&((*handler)->header), (*handler)->num_collumns);
    free((*handler));
    *handler = NULL;
}
