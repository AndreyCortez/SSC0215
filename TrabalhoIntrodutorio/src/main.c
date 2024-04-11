#include <csv.h>
#include <dbms.h>

void print_bytes(const void *ptr, size_t size)
{
    const unsigned char *bytes = ptr;
    for (size_t i = 0; i < size; i++)
    {
        printf("%02X ", bytes[i]);
    }
    printf("\n");
}

int main()
{
    FILE *file = fopen("data/FIFA17_official_data.csv", "r");
    if (file == NULL)
    {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    // Chamar a função para analisar o CSV
    CSV_handler *hand = csv_parse(file, true);
    csv_print_head(hand);

    
    const char * collumns_retrieved[] = {"Club", "GKReflexes"};
    CSV_handler* new_hand = csv_retrieve_collumns(hand, collumns_retrieved, 2);
    csv_print_head(new_hand);
    
    // printf("%s", hand->data[0][0]);

    for (int i = 0; i < hand->num_collumns; i++)
    {
        printf("%s\n", hand->header[i]);
    }

    char **col_data = csv_retrieve_collumn(hand, "Club");

    if (col_data != NULL)
    {
        printf("coluna encontrada!");
        for (int i = 0; i < hand->num_rows; i++)
        {
            //printf("%s\n", col_data[i]);
        }
    }

    
    const char *data_to_format[] = {"12", "12.0", "Tamanho arbitrario"};
    const char *format_string = "%d %f %s";
    
    int tam = format_len(format_string, data_to_format);

    printf("\ntamanho dos dados: %d\n", tam);
    void *formated = format_data(format_string, data_to_format);
    print_bytes(formated, tam);

    printf("%d %f ", *((int *)formated), *((float *)(formated + sizeof(int))));
    // for (int i)
    printf("%s\n", (char *)(formated + 12));

    printf("testes encerrados\n");
    // Fechar o arquivo
    fclose(file);

    return 0;
}