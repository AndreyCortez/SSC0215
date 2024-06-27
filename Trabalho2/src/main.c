#include "csv.h"
#include "dbms.h"
#include "funcoes_fornecidas.h"

// Andrey Cortez Rufino - 11819487
// Francyélio de Jesus Campos Lima - 13676537

// Função ajudante para printar o registro no formato correto
void printar_registro_formatado(void *data, char *format)
{

    char *str = (char *)get_data_in_collumn(data, format, 2);
    if (*str != '\0')
        printf("Nome do Jogador: %s\n", str);
    else
        printf("Nome do Jogador: SEM DADO\n");

    free(str);
    str = (char *)get_data_in_collumn(data, format, 3);
    if (*str != '\0')
        printf("Nacionalidade do Jogador: %s\n", str);
    else
        printf("Nacionalidade do Jogador: SEM DADO\n");

    free(str);
    str = (char *)get_data_in_collumn(data, format, 4);
    if (*str != '\0')
        printf("Clube do Jogador: %s\n", str);
    else
        printf("Clube do Jogador: SEM DADO\n");

    free(str);
    printf("\n");
}

// Função que lê a entrada da maneira adequada ao que é pedido pela
// Especificação
void scanf_formated(char *str)
{
    while (1)
    {

        int iterator = 0;
        char c1 = getchar();

        // Ao iniciar com " sabemos que a entrada se trata de uma
        // string e por isso devem ser lidos todos os caracteres dentro
        // da mesma
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
        // Caso contrário, trata-se de um número ou de um campo NULO
        // Assim, deve ser lido até o primeiro caractere de terminação
        else if (c1 != ' ' && c1 != '\n' && c1 != EOF)
        {
            char c = c1;
            while (c != ' ' && c != '\n' && c != EOF && c != '\r' && c != '\0')
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

// Para os casos das funcionalidades 3 e 5 usamos a seguinte função para
// identificar os tipos de parametros e a que coluna correspondem na tabela
void decodificar_parametros(int **index_parametros, char ***vlr_parametros, int num_parametros)
{
    // Alocamos espaço para armazenar as colunas dos parametros conforme
    // o necessário
    *index_parametros = malloc(sizeof(int) * num_parametros);
    // Iniciamos todos os valores com -1 para evitar comportamentos inesperados
    memset(*index_parametros, -1, num_parametros);

    // Alocamos espaço para os valores dos parametro, 100 caracteres
    // deve ser o suficiente entretanto podem ser usar outras técnicas dinamicas
    // (embora não sejam recomendadas, visto que adicionam complexidade desnecessária)
    *vlr_parametros = malloc(sizeof(char *) * num_parametros);
    for (int i = 0; i < num_parametros; i++)
    {
        (*vlr_parametros)[i] = malloc(sizeof(char) * 100);
    }

    // Laço para decidir o tipo de cada uma das entradas
    for (int j = 0; j < num_parametros; j++)
    {
        char parametro[20];
        scanf("%s", parametro);

        if (strcmp(parametro, "id") == 0)
        {
            char valor_parametro[100];
            scanf("%s", valor_parametro);

            // No caso de um inteiro, o valor de entrada é convertido em um número
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
            // É importante usar essa função para identificar valores nulos
            // e valores que estão entre aspas
            scanf_formated(valor_parametro);

            // No caso de ser do tipo string o valor de entrada é simplesmente copiado para
            // a posição correta
            (*index_parametros)[j] = 2;
            strcpy((*vlr_parametros)[j], valor_parametro);
        }
        else if (strcmp(parametro, "nomeClube") == 0)
        {
            char valor_parametro[100];
            scanf_formated(valor_parametro);

            (*index_parametros)[j] = 4;
            strcpy((*vlr_parametros)[j], valor_parametro);
        }
        else if (strcmp(parametro, "nacionalidade") == 0)
        {
            char valor_parametro[100];
            scanf_formated(valor_parametro);

            (*index_parametros)[j] = 3;
            strcpy((*vlr_parametros)[j], valor_parametro);
        }
    }
}

// Libera as arrays alocadas na função anterior
void liberar_parametros(int **index_parametros, char ***vlr_parametros, int num_parametros)
{
    for (int i = 0; i < num_parametros; i++)
    {
        free((*vlr_parametros)[i]);
    }

    free(*vlr_parametros);
    free(*index_parametros);

    vlr_parametros = NULL;
    index_parametros = NULL;
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

        CSV_handler *csv_handle = csv_parse(csv_path, true);
        if (csv_handle == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }

        Table *new_table = table_create_from_csv(bin_path, csv_handle, (char *)format);
        if (new_table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }

        csv_free_handle(&csv_handle);
        table_free(&new_table);

        binarioNaTela(bin_path);
    }
    // Segundo comando, imprime os dados de um binário
    // entrada: path do binário
    else if (command == 2)
    {
        char bin_path[100];

        scanf("%s", bin_path);
        Table *table = table_access(bin_path, format);

        // A função table_access é capaz de identificar o status do arquivo
        // quando detecta uma falha ela retorna NULL e por isso é importante checar
        if (table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }

        int registers_read = 0;

        // O seguinte laço se move por todos os registros da tabela
        // podemos usar o campo current_register da estrutura table
        // para exibir os dados do registro atual
        while (table_move_to_next_register(table))
        {
            void *current_data = table->current_register.data;
            printar_registro_formatado(current_data, table->format);
            registers_read += 1;
        }

        if (!registers_read)
        {
            printf("Registro inexistente.\n\n");
        }

        table_free(&table);
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

        // Colocamos retroativamente o uso de indice na funcionalidade 3
        table_create_index(table, "auxi.bin", 0, 4);

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

            // Decodifica os parametros de entrada de acordo com o formato especificado
            decodificar_parametros(&parametros, &valor_parametros, num_parametros);

            int num_validos = 0;

            // Cada vez que um match na tabela for encontrado o código que está dentro do while será executado
            // A função table_search_for_matcher detecta automaticamente se o parametro que está sendo procurado é
            // a PK e assim faz a busca usando o indice
            while (table_search_for_matches(table, (void **)valor_parametros, parametros, num_parametros))
            {
                printar_registro_formatado(table->current_register.data, table->format);
                num_validos++;
            }

            // Caso nenhum registro válido seja encotrado, executamos isso
            if (num_validos == 0)
            {
                printf("Registro inexistente.\n\n");
            }

            liberar_parametros(&parametros, &valor_parametros, num_parametros);
        }

        table_free(&table);
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

        // Cria-se um indice para a tabela no caminho especificado
        // a PK é dada pela coluna 0 e seu tamanho é de 4 bytes
        table_create_index(table, index_bin_path, 0, 4);

        table_free(&table);
        binarioNaTela(index_bin_path);
    }
    else if (command == 5)
    {
        char bin_path[100];
        char index_bin_path[100];
        int qtd_buscas;

        scanf("%s %s %d", bin_path, index_bin_path, &qtd_buscas);
        Table *table = table_access(bin_path, format);

        if (table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }

        // Criamos o arquivo de index para a tabela
        table_create_index(table, index_bin_path, 0, 4);


        for (int i = 0; i < qtd_buscas; i++)
        {
            int num_parametros;
            scanf("%d", &num_parametros);

            int *parametros;
            char **valor_parametros;

            // A seguinte função decodifica os parametros de entrada no formatp
            // especificado
            decodificar_parametros(&parametros, &valor_parametros, num_parametros);

            // Cada vez que um match na tabela for encontrado o código que está dentro do while será executado
            // A função table_search_for_matcher detecta automaticamente se o parametro que está sendo procurado é
            // a PK e assim faz a busca usando o indice
            while (table_search_for_matches(table, (void **)valor_parametros, parametros, num_parametros))
            {
                printar_registro_formatado(table->current_register.data, table->format);
            }

            // O indice é recriado a cada vez que há uma alteração na tabela
            table_create_index(table, index_bin_path, 0, 4);

            liberar_parametros(&parametros, &valor_parametros, num_parametros);
        }

        table_free(&table);

        binarioNaTela(bin_path);
        binarioNaTela(index_bin_path);
    }
    else if (command == 6)
    {
        char bin_path[100];
        char index_bin_path[100];
        int qtd_insercoes;

        scanf("%s %s %d", bin_path, index_bin_path, &qtd_insercoes);
        Table *table = table_access(bin_path, format);
        table_create_index(table, index_bin_path, 0, 4);

        if (table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }

        for (int i = 0; i < qtd_insercoes; i++)
        {

            char *parametros[5];
            for (int i = 0; i < 5; i++)
            {
                // Salvamos os parametros numa lista de strings e formatamos
                // no formato especificado
                parametros[i] = malloc(sizeof(char) * 100);
                scanf_formated(parametros[i]);
            }

            // Função que insere um novo registro na tabela
            table_insert_new_row(table, parametros);

            for (int i = 0; i < 5; i++)
                free(parametros[i]);
            

            // Reseta o arquivo indice, agora com o novo valor que foi inserido
            table_create_index(table, index_bin_path, 0, 4);
        }

        table_free(&table);

        binarioNaTela(bin_path);
        binarioNaTela(index_bin_path);
    }
    else if (command == 7)
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

        // Cria-se um indice para a tabela no caminho especificado
        // a PK é dada pela coluna 0 e seu tamanho é de 4 bytes
        table_create_btree(table, index_bin_path, 0, 4);

        table_free(&table);
        binarioNaTela(index_bin_path);
    }
    else if (command == 8)
    {
        char bin_path[100];
        char index_bin_path[100];
        int qtd_buscas;

        scanf("%s %s %d", bin_path, index_bin_path, &qtd_buscas);
        Table *table = table_access(bin_path, format);
        table->btree = btree_acess(index_bin_path);

        if (table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }
        if (table->btree == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }




        for (int i = 0; i < qtd_buscas; i++)
        {
            printf("BUSCA %d\n\n", i + 1);
            int num_parametros = 1;

            int *parametros;
            char **valor_parametros;

            // A seguinte função decodifica os parametros de entrada no formatp
            // especificado
            decodificar_parametros(&parametros, &valor_parametros, num_parametros);
            int num_validos = 0;

            // Cada vez que um match na tabela for encontrado o código que está dentro do while será executado
            // A função table_search_for_matcher detecta automaticamente se o parametro que está sendo procurado é
            // a PK e assim faz a busca usando o indice
            while (table_search_for_matches(table, (void **)valor_parametros, parametros, num_parametros))
            {
                num_validos++;
                printar_registro_formatado(table->current_register.data, table->format);
            }

            
            // Caso nenhum registro válido seja encotrado, executamos isso
            if (num_validos == 0)
            {
                printf("Registro inexistente.\n\n");
            }

            liberar_parametros(&parametros, &valor_parametros, num_parametros);
        }

        table_free(&table);
    }
    else if (command == 9)
    {
        char bin_path[100];
        char ind_bin_path[100];
        int qtd_buscas;

        scanf("%s %s %d", bin_path, ind_bin_path, &qtd_buscas);
        Table *table = table_access(bin_path, format);
        table->btree = btree_acess(ind_bin_path);

        if (table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }
        if (table->btree == NULL)
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

            // Decodifica os parametros de entrada de acordo com o formato especificado
            decodificar_parametros(&parametros, &valor_parametros, num_parametros);

            int num_validos = 0;

            // Cada vez que um match na tabela for encontrado o código que está dentro do while será executado
            // A função table_search_for_matcher detecta automaticamente se o parametro que está sendo procurado é
            // a PK e assim faz a busca usando o indice
            while (table_search_for_matches(table, (void **)valor_parametros, parametros, num_parametros))
            {
                printar_registro_formatado(table->current_register.data, table->format);
                num_validos++;
            }

            // Caso nenhum registro válido seja encotrado, executamos isso
            if (num_validos == 0)
            {
                printf("Registro inexistente.\n\n");
            }

            liberar_parametros(&parametros, &valor_parametros, num_parametros);
        }

        table_free(&table);
    }
    else if (command == 10)
    {
        char bin_path[100];
        char index_bin_path[100];
        int qtd_insercoes;

        scanf("%s %s %d", bin_path, index_bin_path, &qtd_insercoes);
        Table *table = table_access(bin_path, format);
        table->btree = btree_acess(index_bin_path);
        //table_create_btree(table, index_bin_path, 0, 4);

        if (table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }
        if (table->btree == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }

        for (int i = 0; i < qtd_insercoes; i++)
        {

            char *parametros[5];
            for (int i = 0; i < 5; i++)
            {
                // Salvamos os parametros numa lista de strings e formatamos
                // no formato especificado
                parametros[i] = malloc(sizeof(char) * 100);
                scanf_formated(parametros[i]);
            }

            // Função que insere um novo registro na tabela
            table_insert_new_row(table, parametros);

            for (int i = 0; i < 5; i++)
                free(parametros[i]);
            
        }

        table_free(&table);

        binarioNaTela(bin_path);
        binarioNaTela(index_bin_path);
    }
    // Função de debug pra imprimir os registros removidos;
    else if (command == -1)
    {
        char bin_path[100];
        char index_bin_path[100];

        scanf("%s %s", bin_path, index_bin_path);
        Table *table = table_access(bin_path, format);
        table_create_index(table, index_bin_path, 0, 4);
        table->btree = btree_acess(index_bin_path);

        if (table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }
        if (table->btree == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }

        Register reg = register_read(table->top, table->f_pointer);
        while (reg.prox_reg > 0)
        {
            printf("%d\n", reg.tam_reg);
            reg = register_read(reg.prox_reg, table->f_pointer);
        }
        printf("%d\n", reg.tam_reg);

        table_free(&table);

        binarioNaTela(bin_path);
        binarioNaTela(index_bin_path);
    }
    else if (command == -2)
    {
        char bin_path[100];
        char index_bin_path[100];

        scanf("%s %s", bin_path, index_bin_path);
        Table *table = table_access(bin_path, format);
        table_create_index(table, index_bin_path, 0, 4);

        if (table == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            return 0;
        }

        table_create_btree(table, index_bin_path, 0, 4);

        // Btree *bt = table->btree;
        for (int i = 0; i < 4; i++)
        {
            // Bnode *bn = bnode_read(bt, i);
            // printf("cur bnode: %d\n", bn->cur_rrn);
            // printf("    num chaves: %d\n", bn->num_keys);

            // for (int j = 0; j < MAX_KEYS - 1; j++)
            //     printf("    key/off:%d %lld\n", bn->key[j], bn->byte_offset[j]);
            // for (int j = 0; j < MAX_KEYS; j++)
            //     printf("    nextrrn:%d\n", bn->next_rrn[j]);
        
        }


        table_free(&table);

        binarioNaTela(bin_path);
        binarioNaTela(index_bin_path);
    }
    else if (command == -3)
    {
        char index_bin_path[100];

        scanf("%s", index_bin_path);

        // Btree *bt = btree_create(index_bin_path);

        // btree_save_header(bt, '0');

    
        // btree_insert(bt, 1, 1);
        // btree_insert(bt, 2, 1);
        // btree_insert(bt, 3, 1);
        // btree_insert(bt, 4, 1);
        // btree_insert(bt, 5, 1);
        // btree_insert(bt, 6, 1);

        
        // printf("root: %d\n", bt->root);

        for (int i = 0; i < 4; i++)
        {
            // Bnode *bn = bnode_read(bt, i);
            // printf("cur bnode: %d\n", bn->cur_rrn);
            // printf("    num chaves: %d\n", bn->num_keys);
            

            // for (int j = 0; j < bn->num_keys; j++)
            //     printf("    key/off:%d %lld\n", bn->key[j], bn->byte_offset[j]);
            // for (int j = 0; j < bn->num_keys + 1; j++)
            //     printf("    nextrrn:%d\n", bn->next_rrn[j]);
        
        }

        

        // btree_save_header(bt, '1');
        binarioNaTela(index_bin_path);
    }

    return 0;
}
