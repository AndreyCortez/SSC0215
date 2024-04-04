#include <csv.h>

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
    // printf("%s", hand->data[0][0]);

    // Fechar o arquivo
    fclose(file);

    return 0;
}
