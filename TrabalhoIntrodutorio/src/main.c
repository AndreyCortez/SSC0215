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
    
    csv

    // Fechar o arquivo
    fclose(file);

    return 0;
}