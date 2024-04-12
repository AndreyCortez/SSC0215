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

    const char *collumns[] = {"Name", "Age", "Nationality", "Club"};
    const int num_collumns = 4;
    CSV_handler *new_hand = csv_retrieve_collumns(hand, collumns, num_collumns);

    int col = csv_find_collumn(hand, "Club");

    for (int i = 0; i < new_hand->num_rows; i++)
    {
        if (strcmp(new_hand->data[i][3], hand->data[i][col]) != 0)
        {
            printf("errado aqui\n");
            printf("%s\n", hand->data[i][col]);
        }
    }

    csv_print_head(new_hand);

    Table *new_table;
    new_table = table_create_from_csv(new_hand, "%s %d %s %s");

    // Fechar o arquivo
    fclose(file);

    return 0;
}