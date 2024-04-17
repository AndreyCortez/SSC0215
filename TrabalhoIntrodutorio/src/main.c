#include "csv.h"
#include "dbms.h"
#include "funcoes_fornecidas.h"

void print_bytes(const void *ptr, size_t size)
{
    const unsigned char *bytes = ptr;
    for (size_t i = 0; i < size; i++)
    {
        printf("%02X ", bytes[i]);
    }
    printf("\n");
}


// main séria
int main()
{

    int command;

    scanf("%d", &command);

    if (command == 1)
    {    
        char csv_path[100];
        char bin_path[100];
        
        scanf("%s %s", csv_path, bin_path);
        FILE *file = fopen(csv_path, "r");

        if (file == NULL)
        {
            perror("Erro ao abrir o arquivo");
            return 1;
        }

        // Chamar a função para analisar o CSV
        CSV_handler *hand = csv_parse(file, true);
        // csv_print_head(hand);

        // const char *collumns[] = {"Name", "Age", "Nationality", "Club"};
        // const int num_collumns = 4;
        // CSV_handler *new_hand = csv_retrieve_collumns(hand, (char **)collumns, num_collumns);

        const char format[] = "%d %d %s %s %s";
        Table *new_table;
        new_table = table_create_from_csv(hand, (char*)format);

        table_save(new_table, bin_path);
        binarioNaTela(bin_path);

        // Fechar o arquivo
        fclose(file);
    }

    return 0;
}

// 1 dado1.csv jogador.bin
// 1 dado2.csv jogador.bin

// main teste
// int main()
// {
//     FILE *file = fopen("data/FIFA17_official_data.csv", "r");

//     if (file == NULL)
//     {
//         perror("Erro ao abrir o arquivo");
//         return 1;
//     }

//     // Chamar a função para analisar o CSV
//     CSV_handler *hand = csv_parse(file, true);
//     csv_print_head(hand);

//     const char *collumns[] = {"Name", "Age", "Nationality", "Club"};
//     const int num_collumns = 4;
//     const char format[] = "%s %d %s %s";
//     CSV_handler *new_hand = csv_retrieve_collumns(hand, collumns, num_collumns);

//     int col = csv_find_collumn(hand, "Club");

//     for (int i = 0; i < new_hand->num_rows; i++)
//     {
//         if (strcmp(new_hand->data[i][3], hand->data[i][col]) != 0)
//         {
//             printf("errado aqui\n");
//             printf("%s\n", hand->data[i][col]);
//         }
//     }

//     csv_print_head(new_hand);

//     printf("%s %s %s %s\n", new_hand->data[0][0], new_hand->data[0][1], new_hand->data[0][2], new_hand->data[0][3]);
//     int len = format_len(format, new_hand->data[0]);
//     printf("%d\n", len);
//     void *sla_man = format_data(format, new_hand->data[0]);
//     print_bytes(sla_man, len);

//     Table *new_table;
//     new_table = table_create_from_csv(new_hand, "%s %d %s %s");

//     table_save(new_table, "data.bin");

//     // Fechar o arquivo
//     fclose(file);

//     return 0;
// }