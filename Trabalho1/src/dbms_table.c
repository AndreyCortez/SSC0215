#include "dbms_table.h"

// Funções auxiliares para a criação da tabela
// Descrições detalhadas de cada função presentes em suas definições

bool search_using_index(Table *table, void *key);
static void write_table_header(Table *table, char status);


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

    table->search_state = 0;

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

    // Tive que fazer isso pois em alguns casos teste esse valor 
    // foi inicializado errado 
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

    table->search_state = 0;

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

    // Flush para evitar erros imprevisiveis
    fflush(table->f_pointer);

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
    
    // Função que escreve um registro no arquivo
    write_register(reg, table->f_pointer, table->format);
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


// Move o registro atual da tabela para um registro especifico
bool table_goto_register_on_offset(Table *table, int64_t offset)
{
    table->current_register.byte_offset = offset;
    table->current_register.tam_reg = 0;
    return table_move_to_next_register(table);
}

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

    // Libera a memoria do registro atual, caso esteja usando alguma
    free_register(&(table->current_register));
    // lê o registro indicado no next_register_offset
    table->current_register = read_register(next_register_offset, table->f_pointer);

    // Caso o registro esteja removido ele move para o próximo
    if (table->current_register.removed == '1')
        return table_move_to_next_register(table);


    // Retorna o ponteiro do arquivo para onde ele estava antes da função iniciar
    fseek(table->f_pointer, ini_offset, SEEK_SET);
    return true;
}


// A seguinte função é usada para realizar uma busca dentro da tabela, ela vai procurar
// os registros que batem com os valores pedidos para as chaves especificas

// A seguinte função SEMPRE deve ser usada dentro de um laço pois ela envolve estados
// while (table_search_for_matches(...)) {/* Seu código aqui*/}
bool table_search_for_matches(Table *table, void **data, int *indexes, int num_parameters)
{
    if (table->search_state == 0)
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
                    table->search_state = 1;

                return sr;
            }

        // Caso não seja possível usar o indice a busca é feita de item por item
        // Muda-se o estado de busca para 2
        table->search_state = 2;

        // Prepara o ponteiro do current_register para uma nova busca
        table_reset_register_pointer(table);
    }
    else if (table->search_state == 1)
    {
        // Reseta a busca e retorna falso para sair do laço
        table->search_state = 0;
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
    table->search_state = 0;
    return false;
}

bool table_delete_current_register(Table *table)
{
    // Coloca o status da tabela como instável
    write_table_header(table, '0');

    // Começa a alterar o registro  
    table->current_register.removed = '1';
    table->current_register.prox_reg = -1;
    
    // Registros para ajudarem na inserção na lista
    Register before, next;
    before.byte_offset = -1;
    next.byte_offset = -1;
    
    // Caso 0: A lista não possui itens
    if (table->top < 0)
        table->top = table->current_register.byte_offset;
    else
    {
        before = read_register(table->top, table->f_pointer);

        // Caso 1: Inserir no inicio da lista     
        if (before.tam_reg >= table->current_register.tam_reg)
        {
            table->top = table->current_register.byte_offset;
            table->current_register.prox_reg = before.byte_offset;
            goto inserted;
        }
        if (before.prox_reg < 0)
        {
            before.prox_reg = table->current_register.byte_offset;
            goto inserted;
        }

        next = read_register(before.prox_reg, table->f_pointer);

        // Caso 2: Inserir no meio da lista
        while (next.prox_reg > 0)
        {
            if (next.tam_reg > table->current_register.tam_reg)
            {
                before.prox_reg = table->current_register.byte_offset;
                table->current_register.prox_reg = next.byte_offset;
                goto inserted;
            }            
            
            free_register(&before);
            before = next;
            next = read_register(next.prox_reg, table->f_pointer);
        }

        // Caso 3: Inserir no final da lista
        if (next.tam_reg <= table->current_register.tam_reg)
            next.prox_reg = table->current_register.byte_offset;    
        else
        {
            before.prox_reg = table->current_register.byte_offset;
            table->current_register.prox_reg = next.byte_offset;
        }  
    }

inserted:

    if (before.byte_offset != -1) write_register(before, table->f_pointer, table->format);
    if (next.byte_offset != -1) write_register(next, table->f_pointer, table->format);
    write_register(table->current_register, table->f_pointer, table->format);
    
    table->num_removed += 1;
    table->num_reg -= 1;

    write_table_header(table, '1');
    
    free_register(&before);
    free_register(&next);
    
    return true;
}



int64_t find_ptr_to_best_fit(Table *table, int tam)
{
    // Endereço de retorno do ponteiro do arquivo ao fim da execução da função
    int64_t ini_offset = ftell(table->f_pointer);

    // Começamos a iterar a partir do endereço presente no top do header
    int64_t iterator_offset = table->top;
    // Offset anterior ao de iteração, seu primeiro valor é o offset do table top
    int64_t old_offset = 1;

    // Tamanho do refistradir que está sendo iterado sobre
    int tam_cur_reg = -1;

    // Caso o ponteiro seja válido, iteramos sobre ele
    while (iterator_offset > 0)
    {
        // Nos movemos o ponteiro do arquivo para o ponteiro do iterados,
        // pulamos o primeiro bit pois esse é o status de remoção do registro
        fseek(table->f_pointer, iterator_offset + 1, SEEK_SET);

        // Lemos o tamanho do registro
        fread(&tam_cur_reg, sizeof(int32_t), 1, table->f_pointer);

        if (tam_cur_reg >= tam)
        {
            // Caso haja um match, retornamos de imediato, pois os registros 
            // removidos estão em uma lista encadeada crescente
            fseek(table->f_pointer, ini_offset, SEEK_SET);
            return old_offset;
        }

        // Caso contrário salvamos o valor do offset antigo e movemos para o proximo
        old_offset = ftell(table->f_pointer);
        fread(&iterator_offset, sizeof(int64_t), 1, table->f_pointer);
    }

    // Retornando o ponteiro ao seu estado inicial
    fseek(table->f_pointer, ini_offset, SEEK_SET);

    // Caso não exita espaço na tabela para o registro
    // Retonamos -1 que é especial 
    return -1;
}

bool table_insert_new_row(Table *table, char **row)
{
    // valor para onde o offset do arquivo retorna após a execução da função
    int64_t ini_offset = ftell(table->f_pointer);

    // Status do arquivo é colocado como 0
    write_table_header(table, '0');

    // Tamanho do registrador que vai precisar ser inserido
    int data_size = format_len(table->format, row) + register_header_size;
    // Compactamos os dados no formato correto antes de inseri-los
    void *data = format_data(table->format, row);

    // Endereço onde está armazenado o byteoffset do bestfit
    // É necessário fazer isso para realizar a remoção na lista encadeada
    int64_t before_best = find_ptr_to_best_fit(table, data_size);
        
    Register new_register;
    new_register.data = data;
    new_register.prox_reg = -1;
    new_register.removed = '0';

    // Nesse caso não há registros removidos que caibam os dados
    // anexamos então o novo registro ao final da lista
    if (before_best < 0)
    {
        new_register.byte_offset = table->top;
        new_register.tam_reg = data_size;
        table_append_register(table, new_register);
    }
    else
    {
        int64_t best_fit;
        fseek(table->f_pointer, before_best, SEEK_SET);
        fread(&best_fit, sizeof(int64_t), 1, table->f_pointer);

        new_register.byte_offset = best_fit;

        fseek(table->f_pointer, best_fit + 1, SEEK_SET);
        fread(&(new_register.tam_reg), sizeof(int), 1, table->f_pointer);
        
        // endereço do registrado next ao best
        int64_t after_best;
        fread(&after_best, sizeof(int64_t), 1, table->f_pointer);

        fseek(table->f_pointer, before_best, SEEK_SET);
        fwrite(&after_best, sizeof(int64_t), 1, table->f_pointer);

        // caso especial onde o before_best é o table top
        if (before_best == 1)
            table->top = after_best;

        write_register(new_register, table->f_pointer, table->format);
        table->num_removed -= 1;
        table->num_reg += 1;
    }

    // Colocamos o status do arquivo como consitente
    write_table_header(table, '1');

    // Retornamos ao estado inicial
    fseek(table->f_pointer, ini_offset, SEEK_SET);

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
    free_register(&((*tab)->current_register));
    if ((*tab)->f_pointer)
        fclose((*tab)->f_pointer);
    free(*tab);
    (*tab) = NULL;
}