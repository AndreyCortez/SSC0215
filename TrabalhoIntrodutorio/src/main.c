#include "csv.h"
#include "dbms.h"
#include "funcoes_fornecidas.h"

void printar_registro_formatado(void *data)
{
    void *aux_data = data;
    aux_data += sizeof(int) * 2;
    int32_t tam_nome;
    memcpy(&tam_nome, aux_data, sizeof(tam_nome));
    aux_data += sizeof(tam_nome);

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

void ler_string_entre_aspas(char *str) {
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


int main()
{
    const char format[] = "%d %d %s %s %s";
    
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

        CSV_handler *hand = csv_parse(file, true);
        
        Table *new_table;
        new_table = table_create_from_csv(hand, (char*)format);

        table_save(new_table, bin_path);
        binarioNaTela(bin_path);

        csv_free_handle(&hand);
        table_free(&new_table); 
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
            printar_registro_formatado(current_data);
            registers_read += 1;
        }

        if(!registers_read)
        {
            printf("Registro inexistente.\n\n");
        }

        table_free(&table);
    }
    else if (command == 3)
    {
        char bin_path[100];
        int qtd_buscas;
        
        scanf("%s %d", bin_path, &qtd_buscas);
        Table* table = table_access(bin_path);

        if (table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }

        for (int i = 0; i < qtd_buscas; i++)
        {
            table_reset_register_pointer(table);
            printf("Busca %d\n\n", i + 1);

            int num_parametros;
            scanf("%d", &num_parametros);

            int parametros[5] = {-1, -1, -1, -1};
            char parametro_valor[5][100];

            for (int j = 0; j < num_parametros; j++)
            {
                char parametro_id[] = "id";
                char parametro_idade[] = "idade";
                char parametro_nome_jogador[] = "nomeJogador";
                char parametro_nome_clube[] = "nomeClube";
                char parametro_nacionalidade[] = "nacionalidade";

                char parametro[20];
                scanf("%s", parametro);

                if (strcmp(parametro, parametro_id) == 0)
                {
                    char valor_parametro[100];
                    scanf("%s", valor_parametro);

                    int val = atoi(valor_parametro);
                    parametros[j] = 0;
                    memcpy(parametro_valor[j], &val, sizeof(val));
                }
                else if (strcmp(parametro, parametro_idade) == 0)
                {
                    char valor_parametro[100];
                    scanf("%s", valor_parametro);

                    int val = atoi(valor_parametro);
                    parametros[j] = 1;
                    memcpy(parametro_valor[j], &val, sizeof(val));
                }
                else if (strcmp(parametro, parametro_nome_jogador) == 0)
                {
                    char valor_parametro[100];
                    ler_string_entre_aspas(valor_parametro);

                    parametros[j] = 2;
                    strcpy(parametro_valor[j], valor_parametro);
                }
                else if (strcmp(parametro, parametro_nome_clube) == 0)
                {
                    char valor_parametro[100];
                    ler_string_entre_aspas(valor_parametro);

                    parametros[j] = 3;
                    strcpy(parametro_valor[j], valor_parametro);
                }
                else if (strcmp(parametro, parametro_nacionalidade) == 0)
                {
                    char valor_parametro[100];
                    ler_string_entre_aspas(valor_parametro);

                    parametros[j] = 4;
                    strcpy(parametro_valor[j], valor_parametro);
                }
                
            }

            int num_validos = 0;

            while (table_move_to_next_register(table))
            {
                int iterator = 0;
                void* current_data = table->current_register.data;

                bool valido = true;

                int offset_id = 0;
                    
                int offset_idade = sizeof(int);
                
                int tamanho_nome_jogador = *(int *)(current_data + offset_idade + sizeof(int));
                int offset_nome_jogador = offset_idade + 2 * sizeof(int);

                int tamanho_nacionalidade = *(int *)(current_data + offset_nome_jogador + tamanho_nome_jogador);
                int offset_nacionalidade  = offset_nome_jogador + tamanho_nome_jogador + sizeof(int);
                
                int tamanho_nome_clube = *(int *)(current_data +  offset_nacionalidade + tamanho_nacionalidade);
                int offset_nome_clube = offset_nacionalidade + tamanho_nacionalidade + sizeof(int);

                int id_jogador = *(int *)(current_data + offset_id);
                
                int idade_jogador = *(int *)(current_data + offset_idade);
                
                char nome_jogador[100];
                memcpy(nome_jogador, current_data + offset_nome_jogador, tamanho_nome_jogador);
                nome_jogador[tamanho_nome_jogador] = '\0';

                char nome_clube[100];
                memcpy(nome_clube, current_data + offset_nome_clube, tamanho_nome_clube);
                nome_clube[tamanho_nome_clube] = '\0';

                char nacionalidade[100];
                memcpy(nacionalidade, current_data + offset_nacionalidade, tamanho_nacionalidade);
                nacionalidade[tamanho_nacionalidade] = '\0';

                while (parametros[iterator] != -1 && iterator < 5)
                { 

                    if (parametros[iterator] == 0)
                    {
                        //printf("id %d %d\n", *(int *)parametro_valor[iterator],  id_jogador);
                        if (*(int *)parametro_valor[iterator] !=  id_jogador)
                            valido = false;
                
                    } else if (parametros[iterator] == 1)
                    {
                        //printf("idade %d %d\n", *(int *)parametro_valor[iterator],  idade_jogador);
                        if (*(int *)parametro_valor[iterator] !=  idade_jogador)
                            valido = false;
                        
                    } else if (parametros[iterator] == 2)
                    {   
                        if (strcmp(parametro_valor[iterator],  nome_jogador) != 0)
                            valido = false;
                        
                    } else if (parametros[iterator] == 3)
                    {
                        //printf("%s %s\n", parametro_valor[iterator],  nome_clube);
                        if (strcmp(parametro_valor[iterator],  nome_clube) != 0)
                            valido = false;
                        
                    } else if (parametros[iterator] == 4)
                    {
                        //printf("%s %s\n", parametro_valor[iterator],  nacionalidade);
                        if (strcmp(parametro_valor[iterator],  nacionalidade) != 0)
                            valido = false;
                        
                    } 

                    iterator += 1;
                }

                if (valido)
                {
                    printar_registro_formatado(current_data);
                    num_validos += 1;
                }
            }

            if (num_validos == 0)
            {
                printf("Registro inexistente.\n\n");
            }

            table_free(&table);
        }
    }

    return 0;
}
