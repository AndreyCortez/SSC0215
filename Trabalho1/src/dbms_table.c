#include "dbms_table.h"

// Funções auxiliares para a criação da tabela
// Descrições detalhadas de cada função presentes em suas definições

bool search_using_index(Table *table, void *key);
void write_table_header(Table *table, char status);


// Cria a tabela
Table *table_create(char* path, char ***raw_data, char *format, int num_rows)
{
    FILE *file = fopen(path, "r+b");

    // Caso o arquivo não exista ele é criado
    if (file == NULL) {
        file = fopen(path, "w+b");

        // Caso isso falhe retorna NULL
        if (file == NULL) 
            return NULL;
    }

    // Inicializa espaço na memória para uma nova tabela
    Table *table;
    table = (Table *)malloc(sizeof(Table));

    // Faz a decodificação do formato das colunas da tabela 
    table->status = '0';
    table->format = decode_format(format);
    table->top = -1;
    table->next_byte_offset = table_header_size;
    table->num_reg = 0;
    table->num_removed = 0;
    table->index_loaded = false;
    table->f_pointer = file;

    write_table_header(table, '0');

    for (int i = 0; i < num_rows; i++)
    {
        Register reg;
        reg.removed = '0';
        reg.tam_reg = format_len(table->format, raw_data[i]) + register_header_size;
        reg.data = format_data(table->format, raw_data[i]);
        reg.prox_reg = -1;
        table_append_register(table, reg);
        free(reg.data);
    }

    
    write_table_header(table, '1');
    table_reset_register_pointer(table);

    // Seta os valores referentes ao indice de PK
    table->index_loaded = false;
    table->index.key = NULL;
    table->index.byte_offset = NULL;

    return table;
}

// Essa função é apenas um wrapper da função table_create que funciona junto com a biblioteca
// csv.c é colocada para o usuário como uma alternativa de uso (É a alternativa que nós escolhemos)
Table *table_create_from_csv(char* path, CSV_handler *handler, char *format)
{
    return table_create(path, handler->data, format, handler->num_rows);
}

// Acessa um arquivo com os dados da tabela
Table *table_access(char *path, char *format)
{
    FILE *file = fopen(path, "r+b");

    if (file == NULL)
        return NULL;

    // Inicializa a tabela assim como na função table create
    Table *table = malloc(sizeof(Table));
    table->f_pointer = file;
    table->format = decode_format(format);

    // Checa o status da tabela para verificar a integridade do arquivo
    fread(&(table->status), sizeof(char), 1, file);
    if (table->status == '0')
        return NULL;

    fread(&(table->top), sizeof(table->top), 1, file);
    fread(&(table->next_byte_offset), sizeof(table->next_byte_offset), 1, file);

    // Tive que fazer isso pois em alguns casos teste esse valor estava errado 
    int64_t c = ftell(table->f_pointer);
    fseek(table->f_pointer, 0, SEEK_END);
    table->next_byte_offset = ftell(table->f_pointer);
    fseek(table->f_pointer, c, SEEK_SET);

    fread(&(table->num_reg), sizeof(table->num_reg), 1, file);
    fread(&(table->num_removed), sizeof(table->num_removed), 1, file);

    table_reset_register_pointer(table);
    table->index_loaded = false;
    table->index.key = NULL;
    table->index.byte_offset = NULL;

    return table;
}


// Reescreve ou escreve o cabeçalho da tabela
void write_table_header(Table *table, char status)
{
    // Salva o offset de onde o arquivo estava antes de entrar na função
    // Nesse caso é util pois essa função é também utilizada para 
    // setar o status do arquivo
    int64_t ini_offset = ftell(table->f_pointer);

    // Movemos o ponteiro de arquivo para o inicio dele
    fseek(table->f_pointer, 0, SEEK_SET);

    // O char status é uma das entradas e indica o status da tablela
    // caso o status seja '0' é a primeira coisa a ser modificada nela
    // pois indica que a partir daqui ocorrerão outras modificações 
    fputc('0', table->f_pointer);
    table->status = '0';

    fwrite(&(table->top), sizeof(table->top), 1, table->f_pointer);
    fwrite(&(table->next_byte_offset), sizeof(table->next_byte_offset), 1, table->f_pointer);
    fwrite(&(table->num_reg), sizeof(table->num_reg), 1, table->f_pointer);
    fwrite(&(table->num_removed), sizeof(table->num_removed), 1, table->f_pointer);

    // Caso o status seja 1 é necessário que ele seja modificado apenas no fim
    // da escrita na tabela, pois apenas a partir daqui não haverão mais escritas
    if (status == '1')
    {
        fseek(table->f_pointer, 0, SEEK_SET);
        fputc('1', table->f_pointer);
        table->status = '1';
    }

    // No final da execução o ponteiro do arquivo volta para onde ele estava antes
    fseek(table->f_pointer, ini_offset, SEEK_SET);

    return;
}

// Anexa um registro ao final da tabela
bool table_append_register(Table *table, Register reg)
{
    int64_t ini_offset = ftell(table->f_pointer);

    //espera-se que o cabeçalho do arquivo esteja com status '0'
    if (table->status != '0')
        return false;
    
    // Para anexar um registro é necessário estar no fim do arquivo
    fseek(table->f_pointer, table->next_byte_offset, SEEK_SET);

    // Escreve o registro
    fwrite(&(reg.removed), sizeof(reg.removed), 1, table->f_pointer);
    fwrite(&(reg.tam_reg), sizeof(reg.tam_reg), 1, table->f_pointer);
    fwrite(&(reg.prox_reg), sizeof(reg.prox_reg), 1, table->f_pointer);

    // Quando vamos anexar um registro no final do arquivo o tamanho
    // do seu campo de dados é equivalente ao seu tamanho menos o tamanho de seu cabeçalho
    fwrite(reg.data, sizeof(char), reg.tam_reg - register_header_size, table->f_pointer);
    table->num_reg += 1;

    // Ao anexar a tabela recebe um novo top
    table->next_byte_offset = ftell(table->f_pointer);
    write_table_header(table, '0');

    // No final da execução o ponteiro do arquivo volta para onde ele estava antes
    fseek(table->f_pointer, ini_offset, SEEK_SET);
    return true;
}


// As próximas duas funções permitem acessar os registros da tabela de forma sequencial
// Usando a variavel current_register podemos ler e alterar os dados do registro
void table_reset_register_pointer(Table *table)
{
    fseek(table->f_pointer, table_header_size, SEEK_SET);
    table->current_register.byte_offset = ftell(table->f_pointer);
    table->current_register.tam_reg = 0;
}


bool table_goto_register_on_offset(Table *table, int64_t offset)
{
    table->current_register.byte_offset = offset;
    table->current_register.tam_reg = 0;
    return table_move_to_next_register(table);
}

// Essa variavél serve para armazenar os dados do current_register que podem ser de 
// tamanho variável, ela é liberada toda vez que movemos para o próximo registrador
// ou seja, não precisamos nos preocupar com ela
void *temp_reg_data = NULL;

// Função que lê o próximo registrador e salva na variável next_register
bool table_move_to_next_register(Table *table)
{
    // offset de retorno do f_pointer quando a função terminar de ser executada
    int64_t ini_offset = ftell(table->f_pointer);

    // verifica a posição do proximo registrador
    int64_t next_register_offset = table->current_register.byte_offset + table->current_register.tam_reg;

    // printf("%d %d\n",next_register_offset, table->next_byte_offset);
    // Caso o proximo registrador esteja no final do arquivo a função retorna false
    if (next_register_offset >= table->next_byte_offset)
    {
        // Retorna o ponteiro do arquivo para onde ele estava antes da função iniciar
        fseek(table->f_pointer, ini_offset, SEEK_SET);
        return false;
    }

    // Acessa a posição do proximo registrador
    fseek(table->f_pointer, next_register_offset, SEEK_SET);

    Register reg;

    // Lê os dados do proximo registrador
    reg.byte_offset = ftell(table->f_pointer);
    fread(&(reg.removed), sizeof(reg.removed), 1, table->f_pointer);
    fread(&(reg.tam_reg), sizeof(reg.tam_reg), 1, table->f_pointer);
    fread(&(reg.prox_reg), sizeof(reg.prox_reg), 1, table->f_pointer);

    int data_size = reg.tam_reg - register_header_size;

    if (temp_reg_data != NULL)
        free(temp_reg_data);
    
    // Aloca dados conforme o necessário
    temp_reg_data = malloc(data_size);
    fread(temp_reg_data, sizeof(char), data_size, table->f_pointer);
    reg.data = temp_reg_data;

    // Seta o current_register pra ser esse registro definido ali em cima
    table->current_register = reg;

    // Caso o registro esteja removido ele move para o próximo
    if (table->current_register.removed == '1')
        return table_move_to_next_register(table);


    // Retorna o ponteiro do arquivo para onde ele estava antes da função iniciar
    fseek(table->f_pointer, ini_offset, SEEK_SET);
    return true;
}



// Reescreve o registro que está sendo atualmente acessado
bool rewrite_current_register(Table *table)
{
    fseek(table->f_pointer, -table->current_register.tam_reg, SEEK_CUR);
    //write_current_register(table->f_pointer, table->current_register);
    fseek(table->f_pointer, -register_header_size, SEEK_CUR);
    return true;
}


// A seguinte função é usada para realizar uma busca dentro da tabela, ela vai procurar
// os registros que batem com os valores pedidos para as chaves especificas

// Essa variável decide o estado da busca dos intens dentro da tabela
// 0 : Uma busca não esta em progresso ou a busca com indice retornou false
// 1 : Uma busca acabou de ser realizada usando indice e foi bem sucessedida
// 2 : Uma busca está em progresso
int search_state = 0;

// A seguinte função SEMPRE deve ser usada dentro de um laço pois ela envolve estados
// while (table_search_for_matches(...)) {/* Seu código aqui*/}
bool table_search_for_matches(Table *table, void **data, int *indexes, int num_parameters)
{
    if (search_state == 0)
    {

        // Checa se o indice da PK está presente entre as chaves 
        // e se o arquivo de indices está carregado
        for (int i = 0; i < num_parameters; i++)
            if (indexes[i] == 0 && table->index_loaded)
            {  
                // caso esteja ele faz uma busca usando o indice
                bool sr = search_using_index(table, data[i]);
                
                // caso a busca tenha dado certo ele muda para o estado 1 e retorna verdadeiro
                // para que a função execute um laço uma vez antes de sair dele
                if (sr)
                    search_state = 1;

                return sr;
            }

        // Caso não seja possível usar o indice a busca é feita de item por item
        // Muda-se o estado de busca para 2
        search_state = 2;

        // Prepara o ponteiro do current_register para uma nova busca
        table_reset_register_pointer(table);
    }
    else if (search_state == 1)
    {
        // Reseta a busca e retorna falso para sair do laço
        search_state = 0;
        return false;
    }
    // Mesma lógica para a funcionalidade 2 que acessa todos on indices
    while (table_move_to_next_register(table))
    {
        bool match = true;

        for (int i = 0; i < num_parameters; i++)
        {
            // Aqui ele decodifica os dados empacotados retornando o valor presente na coluna
            // desejada
            void *v = get_data_in_collumn(table->current_register.data, table->format, indexes[i]);
            void *value = v;
            void *data_temp = data[i];

            // Caso o valor da coluna seja do tipo string é feita uma comparação de strings
            if (table->format[indexes[i]] == 's')
            {
                if (strcmp(data_temp, value) != 0)
                {
                    match = false;
                    break;
                }
            }
            // Caso seja do tipo inteiro, é feita uma comparação de inteiros
            else if (table->format[indexes[i]] == 'd')
            {
                if (*(int32_t *)data[i] != *(int32_t *)value)
                {
                    match = false;
                    break;
                }
            }

            // Liberamos a memoria alocada para não causar um leak
            free(v);
        }

        // Caso tenha sido encontrado algo o retorno é true, caso contrário ele só 
        // checa o próximo registro
        if (match)
            return true;
    }

    // Chegamos ao fim do arquivo, aqui resetamos o estado da busca e interrompemos o laço
    search_state = 0;
    return false;
}

bool table_delete_current_register(Table *table)
{

    write_table_header(table, '0');
    table->current_register.removed = '1';

    int64_t aux = table->top;
    table->top = table->current_register.byte_offset;
    table->current_register.prox_reg = aux;

    table->num_removed += 1;
    table->num_reg -= 1;

    rewrite_current_register(table);
    write_table_header(table, '1');

    return true;
}



int64_t find_ptr_to_best_fit(Table *table, int tam)
{
    int64_t cur_offset = ftell(table->f_pointer);

    int64_t aux_offset = table->top;
    int64_t old_offset = 1;
    // printf("%lld\n", table->top);

    int best_fit = INT32_MAX;
    int aux_fit = -1;

    while (aux_offset > 0)
    {
        fseek(table->f_pointer, aux_offset + 1, SEEK_SET);
        fread(&aux_fit, sizeof(int32_t), 1, table->f_pointer);

        int64_t ret_offset = ftell(table->f_pointer);
        fread(&aux_offset, sizeof(int64_t), 1, table->f_pointer);

        // getchar();

        aux_fit -= tam;
        //printf("%lld %d %d\n", aux_offset, best_fit, aux_fit);

        if ((aux_fit >= 0 && aux_fit <= best_fit))
        {
            best_fit = aux_fit;
            fseek(table->f_pointer, cur_offset, SEEK_SET);
            return old_offset;
        }

        old_offset = ret_offset;
    }

    // printf("%lld\n", ret_offset);

    fseek(table->f_pointer, cur_offset, SEEK_SET);
    return -1;
}

bool table_insert_new_register(Table *table, char **row)
{
    write_table_header(table, '0');

    int data_size = format_len(table->format, row) + register_header_size;
    // printf("%d\n", data_size);
    void *data = format_data(table->format, row);

    int64_t cur_offset = ftell(table->f_pointer);

    int64_t before_best = find_ptr_to_best_fit(table, data_size);
    int64_t best_fit;

    int tam_reg;

    if (before_best < 0)
    {
        best_fit = table->next_byte_offset;
        table->next_byte_offset += data_size;
        tam_reg = data_size;
    }
    else
    {
        int64_t next;
        fseek(table->f_pointer, before_best, SEEK_SET);
        fread(&next, sizeof(int64_t), 1, table->f_pointer);
        //printf("%d\n", next);

        int64_t current = next;

        fseek(table->f_pointer, next + 1, SEEK_SET);
        fread(&tam_reg, sizeof(int), 1, table->f_pointer);
        fread(&next, sizeof(int64_t), 1, table->f_pointer);

        fseek(table->f_pointer, before_best, SEEK_SET);
        fwrite(&next, sizeof(int64_t), 1, table->f_pointer);
        if (before_best == 1)
            table->top = next;

        fseek(table->f_pointer, current + 1, SEEK_SET);
        //printf("%d\n", tam_reg);

        table->num_removed -= 1;
        best_fit = current;
    }

    fseek(table->f_pointer, best_fit, SEEK_SET);

    // Register new_register;
    // new_register.data = data;
    // new_register.prox_reg = -1;
    // new_register.removed = '0';
    // new_register.tam_reg = tam_reg;

    //write_register_header(table->f_pointer, new_register);
    fwrite(data, sizeof(char), data_size - register_header_size, table->f_pointer);

    int remainig_space = tam_reg - data_size;

    while (remainig_space > 0)
    {
        fputc('$', table->f_pointer);
        remainig_space -= 1;
    }

    table->num_reg += 1;
    write_table_header(table, '1');

    fseek(table->f_pointer, cur_offset, SEEK_SET);

    return true;
}

// Cria um indice de chave, de tamanho key_size, na coluna key_row 
bool table_create_index(Table *table, char *path, int key_row, int key_size)
{
    // offset de retorno do f_pointer quando a função terminar de ser executada
    int64_t ini_offset = ftell(table->f_pointer);

    // Código padrão para checar a integridade do arquivo
    FILE *file;
    file = fopen(path, "w+b");

    if (file == NULL)
        return false;
    
    // Resetamos o ponteiro de registro da tabela para poder percorrer ela por inteiro
    table_reset_register_pointer(table);
    // Setamos o status do arquivo como 0 pois estamos alterando ele
    fseek(file, 0, SEEK_SET);
    fputc('0', file);

    // Movemos por todos os registros válidos da tabela, salvando o valor de sua chave e 
    // seu byte offset
    while (table_move_to_next_register(table))
    {
        fwrite(get_data_in_collumn(table->current_register.data, table->format, key_row), sizeof(char), key_size, file);
        fwrite(&(table->current_register.byte_offset), sizeof(int64_t), 1, file);
    }

    // Setamos o status do arquivo como 1 pois não estamos mais usando ele
    fseek(file, 0, SEEK_SET);
    fputc('1', file);
    fflush(file);

    // Fechando o arquivo
    fclose(file);

    // Carregamos o arquivo ao usar essa função para evitar repetição de código
    // Criamos uma função de load index para que o usuário carregue um arquivo 
    // de indice já existente caso ele queira
    table_load_index(table, path, key_row, key_size);

    // Retorna o ponteiro do arquivo para onde ele estava antes da função iniciar
    fseek(table->f_pointer, ini_offset, SEEK_SET);

    return true;
}

// Função que carrega o indice
bool table_load_index(Table *table, char *path, int key_row, int key_size)
{
    // Abre o arquivo
    FILE *file;
    file = fopen(path, "r+b");

    if (file == NULL)
        return false;

    // Como vamos ler e escrever no arquivo em sequencia é importante
    // dar o fflush para evitar race conditions
    fputc('0', file);
    fflush(file);
    rewind(file);
    fseek(file, 1, SEEK_SET);

    // Checa se a lista de chave do arquivo é nula
    if (table->index.key != NULL)
    {
        // Caso não seja ele libera esse espaço da memória e coloca
        // ela como nula
        free(table->index.key);
        table->index.key = NULL;
    }

    // Checa se a lista da byte_offset é nula
    if (table->index.byte_offset != NULL)
    {
        // Se não for, esse espaço é liberado e ela é colocada em nula
        free(table->index.byte_offset);
        table->index.byte_offset = NULL;
    }

    // Guardamos dados do indice dentro da estrutura
    table->index.key_size = key_size;
    table->index.key_row = key_row;
    
    // Alocamos o espaço necessário para as chaves
    table->index.key = malloc(sizeof(void *) * table->num_reg);

    for (int i = 0; i < table->num_reg; i++)
    {
        table->index.key[i] = malloc(table->index.key_size);
    }

    // Alocamos o espaço necessário para os byteoffsets
    table->index.byte_offset = malloc(sizeof(int64_t) * table->num_reg);

    // Lemos os dados do arquivo
    for (int i = 0; i < table->num_reg; i++)
    {
        fread((table->index.key[i]), sizeof(char), table->index.key_size, file);
        fread(&(table->index.byte_offset[i]), sizeof(int64_t), 1, file);
    }
    
    // Damos um flush para evitar race conditions novamente e reescrevemos o status
    fflush(file);
    rewind(file);
    fputc('1', file);

    fclose(file);    

    // Colocamos a tabela com status e indice carregado
    // Agora as buscas com indice acontecerão automaticamente
    table->index_loaded = true;
    return true;
}

// Essa função é bem simples, passa por todos a coluna de PK comparando
// com o resultado desejado, caso tenha sido achado ela retorna true;
bool search_using_index(Table *table, void *key)
{

    for (int i = 0; i < table->num_reg; i++)
    {
        // Checa a igualdade entre os indices
        if (*((int32_t *)(table->index.key[i])) == *((int32_t *)key))
        {
            return table_goto_register_on_offset(table, table->index.byte_offset[i]);
        }
    }

    return false;
}

// Função usada para liberar a estrutura da tabela
void table_free(Table **tab)
{
    if ((*tab)->f_pointer)
        fclose((*tab)->f_pointer);
    free(*tab);
    (*tab) = NULL;
}