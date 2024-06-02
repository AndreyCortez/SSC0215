#include "csv.h"
#include "dbms.h"
#include "funcoes_fornecidas.h"

// Andrey Cortez Rufino - 11819487
// Francyélio de Jesus Campos Lima - 13676537

// Função ajudante para printar o registro no formato correto
void printar_registro_formatado(void *data)
{
    void *aux_data = data;
    aux_data += sizeof(int) * 2;
    int32_t tam_nome;
    memcpy(&tam_nome, aux_data, sizeof(tam_nome));
    aux_data += sizeof(tam_nome);

    if (tam_nome != 0)
        printf("Nome do Jogador: %.*s\n", tam_nome, (char *)aux_data);
    else
        printf("Nome do Jogador: SEM DADO\n");

    aux_data += tam_nome;
    memcpy(&tam_nome, aux_data, sizeof(tam_nome));
    aux_data += sizeof(tam_nome);

    if (tam_nome != 0)
        printf("Nacionalidade do Jogador: %.*s\n", tam_nome, (char *)aux_data);
    else
        printf("Nacionalidade do Jogador: SEM DADO\n");

    aux_data += tam_nome;
    memcpy(&tam_nome, aux_data, sizeof(tam_nome));
    aux_data += sizeof(tam_nome);

    if (tam_nome != 0)
        printf("Clube do Jogador: %.*s\n", tam_nome, (char *)aux_data);
    else
        printf("Clube do Jogador: SEM DADO\n");

    printf("\n");
}

// Função que lê strings no meio de aspas
void ler_string_entre_aspas(char *str)
{
    while (1)
    {

        int iterator = 0;
        char c1 = getchar();

        if (c1 == '\"')
        {
            char c = getchar();
            while (c != '\"')
            {
                str[iterator] = c;
                iterator += 1;
                c = getchar();
            }
            str[iterator] = '\0';

            return;
        }
    }
}


void decodificar_parametros(int** index_parametros, char*** vlr_parametros, int num_parametros)
{
    *index_parametros = malloc(sizeof(int) * num_parametros);

    *vlr_parametros = malloc(sizeof(char*) * num_parametros);
    for (int i = 0; i < num_parametros; i++)
    {
        (*vlr_parametros)[i] = malloc(sizeof(char) * 100);
    }

    for (int j = 0; j < num_parametros; j++)
    {
        char parametro[20];
        scanf("%s", parametro);

        if (strcmp(parametro, "id") == 0)
        {
            char valor_parametro[100];
            scanf("%s", valor_parametro);

            int val = atoi(valor_parametro);
            (*index_parametros)[j] = 0;
            memcpy((*vlr_parametros)[j], &val, sizeof(val));
        }
        else if (strcmp(parametro, "idade") == 0)
        {
            char valor_parametro[100];
            scanf("%s", valor_parametro);

            int val = atoi(valor_parametro);
            (*index_parametros)[j] = 1;
            memcpy((*vlr_parametros)[j], &val, sizeof(val));
        }
        else if (strcmp(parametro, "nomeJogador") == 0)
        {
            char valor_parametro[100];
            ler_string_entre_aspas(valor_parametro);

            (*index_parametros)[j] = 2;
            strcpy((*vlr_parametros)[j], valor_parametro);
        }
        else if (strcmp(parametro, "nomeClube") == 0)
        {
            char valor_parametro[100];
            ler_string_entre_aspas(valor_parametro);

            (*index_parametros)[j] = 4;
            strcpy((*vlr_parametros)[j], valor_parametro);
        }
        else if (strcmp(parametro, "nacionalidade") == 0)
        {
            char valor_parametro[100];
            ler_string_entre_aspas(valor_parametro);

            (*index_parametros)[j] = 3;
            strcpy((*vlr_parametros)[j], valor_parametro);
        }
    }
}

// função principal do programa
int main()
{
    // formato dos itens no csv 
    char format[] = "%d %d %s %s %s";

    // comando a ser executado
    int command;
    scanf("%d", &command);

    // Primeiro comando, cria um binário a partir de um csv
    // entrada: path do csv, path do binário
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
        new_table = table_create_from_csv(hand, (char *)format);

        table_save(new_table, bin_path);
        binarioNaTela(bin_path);

        //csv_free_handle(&hand);
        //table_free(&new_table);
        fclose(file);
    }
    // Segundo comando, imprime os dados de um binário
    // entrada: path do binário
    else if (command == 2)
    {
        char bin_path[100];

        scanf("%s", bin_path);
        Table *table = table_access(bin_path, format);

        if (table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }

        int registers_read = 0;
        while (table_move_to_next_register(table))
        {
            void *current_data = table->current_register.data;
            printar_registro_formatado(current_data);
            registers_read += 1;
        }

        if (!registers_read)
        {
            printf("Registro inexistente.\n\n");
        }

        //table_free(&table);
    }
    // Comando de busca
    // entrada primaria: caminho do binario, numero de entradas secundarias
    // entrada secundaria: numero de parametros, parametro, valor do parametro
    else if (command == 3)
    {
        char bin_path[100];
        int qtd_buscas;

        scanf("%s %d", bin_path, &qtd_buscas);
        Table *table = table_access(bin_path, format);
        table_create_index(table, "auxi.bin", 0, 4);
        table_load_index(table, "auxi.bin");

        if (table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }

        for (int i = 0; i < qtd_buscas; i++)
        {
            printf("Busca %d\n\n", i + 1);

            int num_parametros;
            scanf("%d", &num_parametros);

            int *parametros;
            char **valor_parametros;

            decodificar_parametros(&parametros, &valor_parametros, num_parametros);

            int num_validos = 0;

            while (table_search_for_matches(table, (void**) valor_parametros, parametros, num_parametros))
            {
                printar_registro_formatado(table->current_register.data);
                num_validos++;
            }

            if (num_validos == 0)
            {
                printf("Registro inexistente.\n\n");
            }
        }

        //table_free(&table);
    }
    else if (command == 4)
    {
        // 4 binario1.bin index1.bin
        char table_bin_path[100];
        char index_bin_path[100];

        scanf("%s", table_bin_path);
        scanf("%s", index_bin_path);

        Table *table = table_access(table_bin_path, format);
        if (table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }

        table_create_index(table, index_bin_path, 0, 4);

        //table_free(&table);
        binarioNaTela(index_bin_path);
    }
    else if(command == 5)
    {
        char bin_path[100];
        char index_bin_path[100];
        int qtd_buscas;

        scanf("%s %s %d", bin_path, index_bin_path, &qtd_buscas);
        Table *table = table_access(bin_path, format);
        table_create_index(table, index_bin_path, 0, 4);
        table->has_index = false;

        if (table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }

        for (int i = 0; i < qtd_buscas; i++)
        {
            printf("Busca %d\n\n", i + 1);

            int num_parametros;
            scanf("%d", &num_parametros);

            int *parametros;
            char **valor_parametros;

            decodificar_parametros(&parametros, &valor_parametros, num_parametros);

            int num_validos = 0;

            while (table_search_for_matches(table, (void**) valor_parametros, parametros, num_parametros))
            {
                table_delete_current_register(table->current_register.data);
                num_validos++;
            }

            if (num_validos == 0)
            {
                printf("Registro inexistente.\n\n");
            }

            table_create_index(table, index_bin_path, 0, 4);
            table_load_index(table, index_bin_path);

            binarioNaTela(bin_path);
            binarioNaTela(index_bin_path);
        }
    }

    return 0;
}
