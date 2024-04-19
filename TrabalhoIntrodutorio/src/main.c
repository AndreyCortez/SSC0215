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

void print_formated_register_data(void *data)
{
    void *aux_data = data;
    aux_data += sizeof(int) * 2;
    int32_t tam_nome;
    memcpy(&tam_nome, aux_data, sizeof(tam_nome));
    aux_data += sizeof(tam_nome);

    // printf("tam_nome: %d\n", tam_nome);
    if (tam_nome != 0)
        printf("Nome do Jogador: %.*s\n", tam_nome, (char*)aux_data);
    else
        printf("Nome do Jogador: SEM DADO\n");
    
    aux_data += tam_nome;
    memcpy(&tam_nome, aux_data, sizeof(tam_nome));
    aux_data += sizeof(tam_nome);

    if (tam_nome != 0)
        printf("Nacionalidade do Jogador: %.*s\n", tam_nome, (char*)aux_data);
    else
        printf("Nacionalidade do Jogador: SEM DADO\n");

    aux_data += tam_nome;
    memcpy(&tam_nome, aux_data, sizeof(tam_nome));
    aux_data += sizeof(tam_nome);

    if (tam_nome != 0)
        printf("Clube do Jogador: %.*s\n", tam_nome, (char*)aux_data);
    else
        printf("Clube do Jogador: SEM DADO\n");

    printf("\n");
}

// main séria
int main()
{

    int command;
    const char format[] = "%d %d %s %s %s";

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

        CSV_handler *hand = csv_parse(file, true);
        
        Table *new_table;
        new_table = table_create_from_csv(hand, (char*)format);

        table_save(new_table, bin_path);
        binarioNaTela(bin_path);

        fclose(file);
    }
    else if (command == 2)
    {
        char bin_path[100];
        
        scanf("%s", bin_path);
        Table* table = table_access(bin_path);

        if (table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }


        int registers_read = 0;
        while (table_move_to_next_register(table))
        {
            void* current_data = table->current_register.data;
            print_formated_register_data(current_data);
            registers_read += 1;
        }

        if(!registers_read)
        {
            printf("Registro inexistente.\n\n");
        }

    }
    else if (command == 3)
    {
        char bin_path[100];
        int qtd_buscas;
        
        scanf("%s %d", bin_path, &qtd_buscas);
        Table* table = table_access(bin_path);

        

        for (int i = 0; i < qtd_buscas; i++)
        {
            int num_parametros;
            scanf ("%d", &num_parametros);

            for (int j = 0; j < num_parametros; j++)
            {
                
            }

            int offset_id = 0;
            int offset_idade = 4;
            int offset_nome_jogador = 8
            
        }
        
    }

    return 0;
}

// 1 dado1.csv jogador.bin
// 1 dado2.csv jogador.bin
// 2 jogador.bin

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