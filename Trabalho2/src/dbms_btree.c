#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "dbms_btree.h"
// dentro de cada pagina, as chaves sao ordenadas em ordem crescente

void swap(int *a, int *b){
    int temp = *a;
    *a = *b;
    *b = temp;

    return;
}

// Criar cabecalho da arqvore b
Btree* btree_create(char *path)
{
    FILE *file = fopen(path, "w+b");

    if (file == NULL)
        return NULL;


    Btree* aux;
    
    // Alocar espaço
    aux = calloc(1, sizeof(Btree));
    aux->root = -1;
    aux->f_pointer = file;
    aux->next_rrn = 0;

    for(int i = 0; i < PAGE_SIZE - 13; i++)
    {
        aux->trash[i] = '$';
    }

    return aux;
}

// Criar pagina(no) da arvore
Bnode* bnode_create()
{
    Bnode* node;
    node = calloc(1, sizeof(Bnode));
    int i;

    // Inicializando os campos da arvore
    for(i = 0; i < MAX_KEYS - 1; i++){
        node->next_rrn[i] = -1;
        node->byte_offset[i] = -1;
        node->key[i] = -1;
    }
    node->next_rrn[MAX_KEYS - 1] = -1;

    return node;
}

// Ler cabecalho de uma arvore ja existente
// ARVOREB *lerCabecalhoArvoreB(FILE *index){
//     ARVOREB *aux = calloc(1, sizeof(ARVOREB));
    
//     fseek(index, 0, SEEK_SET);
//     // Caso haja falha na alocacao, nao insere-se nada no cabecalho
//     if(aux == NULL) return aux;
    
//     // Leitura de todos os campos
//     fread(&(aux->status), sizeof(unsigned char), 1, index);
//     fread(&(aux->noRaiz), sizeof(int), 1, index);
//     fread(&(aux->nroNiveis), sizeof(int), 1, index);
//     fread(&(aux->proxRRN), sizeof(int), 1, index);
//     fread(&(aux->nroChaves), sizeof(int), 1, index);
//     fread(&(aux->lixo), sizeof(char), 55, index);

//     return aux;
// }

// Apos alteracores em uma arvore, estas devem ser salvas no seu respectivo arquivo
bool btree_save_header(Btree *btree, char status)
{
    FILE *index = btree->f_pointer;

    // Apontar para o inicio do arquivo
    fseek(index, 0, SEEK_SET);

    // Mudar o status do arquivo
    btree->status = status;

    // Escrever as informacoes existitentes na arvore (sejam novas, ou nao)
    fwrite(&(btree->status), sizeof(unsigned char), 1, index);
    fwrite(&(btree->root), sizeof(int), 1, index);
    //fwrite(&(btree->max_height), sizeof(int), 1, index);
    fwrite(&(btree->next_rrn), sizeof(int), 1, index);
    fwrite(&(btree->num_keys), sizeof(int), 1, index);

    // Escrever lixo nos lugares pertinentes
    for(int i = 0; i < PAGE_SIZE - 13; i++){
        btree->trash[i] = '$';
    }

    fwrite(&(btree->trash), sizeof(char), 55, index);

    return true;
}

// Ler um pagina ja existente
Bnode *bnode_read(Btree* btree, int RRN){
    int i;
    // Utilizaremos uma pagina auxiliar para receber os valores da leitura
    Bnode *node = bnode_create();
    node->cur_rrn = RRN;
    
    // Ajustar o ponteiro para o RRN correto
    fseek(btree->f_pointer, PAGE_SIZE + (PAGE_SIZE * RRN), SEEK_SET);
    
    // Ler os campos
    fread(&(node->height), sizeof(int), 1, btree->f_pointer);
    fread(&(node->num_keys), sizeof(int), 1, btree->f_pointer);
    
    for(i = 0; i < MAX_KEYS - 1; i++)
    {
        fread(&(node->key[i]), sizeof(int), 1, btree->f_pointer);
        fread(&(node->byte_offset[i]), sizeof(int64_t), 1, btree->f_pointer);
    }
    
    for(i = 0; i < MAX_KEYS; i++)
        fread(&(node->next_rrn[i]), sizeof(int), 1, btree->f_pointer);

    return node;
}

// Guardar uma pagina
int bnode_save(Btree* btree, Bnode* bnode)
{
    // printf("%d\n", bnode->cur_rrn);
    // Ajustar o ponteiro para a posicao correta
    fseek(btree->f_pointer, PAGE_SIZE + (PAGE_SIZE * bnode->cur_rrn), SEEK_SET);

    // Escrever todos os campos
    fwrite(&(bnode->height), sizeof(int), 1, btree->f_pointer);
    fwrite(&(bnode->num_keys), sizeof(int), 1, btree->f_pointer);
    
    for(int i = 0; i < MAX_KEYS - 1; i++)
    {
        fwrite(&(bnode->key[i]), sizeof(int), 1, btree->f_pointer);
        fwrite(&(bnode->byte_offset[i]), sizeof(int64_t), 1, btree->f_pointer);
    }
    
    for(int i = 0; i < MAX_KEYS; i++) 
        fwrite(&(bnode->next_rrn[i]), sizeof(int), 1, btree->f_pointer);

    return 1;
}

// Percorrer o vetor do no para procurar uma chave
int32_t bnode_search_key(Bnode* bnode, int key){
    int *v = bnode->key;
    int tam = bnode->num_keys; 

    for(int i = 0; v[i] <= key && i < tam; i++)
    {
        if(v[i] == key)
            return i;
    }
    
    return -1;
}

// Dividir o vetor de chaves
int split(Bnode *new, Bnode* page, Bnode* newpage, Bnode *promoted, int POS)
{
    int chaves[MAX_KEYS], refs[MAX_KEYS], childs[MAX_KEYS+1];

    // Atribuir uma metada das informacoes para um vetor
    for(int i = 0; i < POS; i ++)
    {
        chaves[i] = page->key[i];
        refs[i] = page->byte_offset[i];
        childs[i] = page->next_rrn[i];
    }

    // Guardar as informacoes do indice do meio
    // Pode dar errado aqui
    childs[POS] = page->next_rrn[POS];
    chaves[POS] = new->key[0];
    refs[POS] = new->byte_offset[0];
    childs[POS + 1] = new->next_rrn[0];
    
    // Atribuir as informacoes da outra metade
    for(int i = POS, j = POS + 1; j < MAX_KEYS; i++, j++){
        chaves[j] = page->key[i];
        refs[j] = page->byte_offset[i];
        childs[j+1] = page->next_rrn[i];
    }

    // Guardar as informacoes importantes, para nao perder a referencia
    promoted->key[0] = chaves[MAX_KEYS/2];
    promoted->byte_offset[0] = refs[MAX_KEYS/2];
    promoted->cur_rrn =  newpage->cur_rrn;

    // As informacoes referenciadas anteriormente serao atribuidas agora
    for(int i = 0; i < MAX_KEYS / 2; i++){
        page->key[i] = chaves[i];
        page->byte_offset[i] = refs[i];
        page->next_rrn[i] = childs[i];
    }

    page->next_rrn[MAX_KEYS / 2] = childs[MAX_KEYS / 2];

    // Inicializar as outras posicoes dos novos vetores para evitar sujeiras
    for(int i = MAX_KEYS / 2; i < MAX_KEYS - 1; i++){
        page->key[i] = -1;
        page->byte_offset[i] = -1;
        page->next_rrn[i+1] = -1;
    }

    // Inicializar as outras posicoes dos novos vetores para evitar sujeiras
    for(int i = (MAX_KEYS / 2) + 1, j = 0; i < MAX_KEYS; j++, i++){
        newpage->key[j] = chaves[i];
        newpage->byte_offset[j] = refs[i];
        newpage->next_rrn[j] = childs[i];
    }
    
    // Atualizar informacoes do pagina
    newpage->next_rrn[MAX_KEYS - (MAX_KEYS / 2)] = childs[MAX_KEYS];
    newpage->height = page->height;
    newpage->num_keys = 2;
    page->num_keys = 3;
    
    return true;
}

// Guardar uma nova chave e suas informacoes
bool inserir(Btree *btree, Bnode *inserted, Bnode *promoted)
{
    Bnode *page, *newpage;
    int32_t pos, retorno;
    
    // printf("%d\n", inserted->cur_rrn);

    // Verificar se a posicao atual esta vazia, caso sim fazer a atribuicao
    if(inserted->cur_rrn == -1){
        promoted->key[0] = inserted->key[0];
        promoted->byte_offset[0] = inserted->byte_offset[0];
        promoted->cur_rrn = -1; 
        // printf("aqui\n"); 
        return true;
    } 
    else
    {
        // Ler a pagina atual e procurar a posicao certa
        page = bnode_read(btree, inserted->cur_rrn);
        pos = bnode_search_key(page, inserted->key[0]);

        // Caso ja exista essa chave, finaliza a operacao
        if(pos != -1) 
        {
            free(page);
            return false;
        }

        int32_t cur_rrn = inserted->cur_rrn;
        // Recursivamente verifica se eh possivel guardar a pagina na posicao atual da recursao
        pos = page->num_keys;
        inserted->cur_rrn = page->next_rrn[pos]; 
        retorno = inserir(btree, inserted, promoted);                    
        
        if(!retorno)
        { 
            free(page);
            return false;
        }
        
        // Caso na pagina atual ainda exista espaco para esta chave, sera escrito neste local
        else if(page->num_keys < MAX_KEYS - 1)
        {
            //printf("%d\n", )
            // Shifita os itens para frente e os insere, esta em uma ordenacao do vetor bastante eficiente para este caso
            for(int i = (page->num_keys - 1); i >= pos; i--)
            {
                swap(&(page->key[i]), &(page->key[i+1]));
                swap(&(page->byte_offset[i]), &(page->byte_offset[i+1]));
                swap(&(page->next_rrn[i+1]), &(page->next_rrn[i+2]));
            }

            // Escrever as informacoes
            page->key[pos] = promoted->key[0];
            page->byte_offset[pos] = promoted->byte_offset[0];
            page->next_rrn[pos + 1] = promoted->cur_rrn;
            page->num_keys++;
            page->cur_rrn = cur_rrn;

            // Salvar pagina
            bnode_save(btree, page); 
            btree->num_keys++;      

            free(page);
            return false;
        
        } 
        // Deu overflow e temos que criar uma nova pagina e apos isso fazer split
        else
        { 
            // Cria pagina
            newpage = bnode_create();
            newpage->cur_rrn = btree->next_rrn;
            page->cur_rrn = inserted->cur_rrn;
            // Faz overflow
            split(inserted, page, newpage, promoted, pos);
            
            // Escreve as novas paginas
            bnode_save(btree, page);
            bnode_save(btree, newpage);
            btree->next_rrn++;

            // Libera as alocacoes
            free(page);
            free(newpage);
            return true;
        }
    }
}

// Recebe um REGISTRO e o insere no arquivo de indice fazendo os balenceamentos necessarios
int32_t btree_insert(Btree *btree, int32_t key, int32_t byte_offset)
{
    Bnode *node;

    // Nao ha nenhum regitro na arvore
    if(btree->root == -1){

        // Cria uma pagina vazia
        node = bnode_create();

        // a primeira pagina sera adicihonada, entao cria-se o primeiro nivel da arvore
        btree->max_height++;
        // o no raiz recebe RRN 0
        btree->root = 0;
        // o proximo RRN eh o 1
        btree->next_rrn = 1;

        // atualizar as informacoes do no que esta sendo inserido na arvore
        node->key[0] = key;
        node->byte_offset[0] = byte_offset;
        node->num_keys++;
        node->height = 1;
        node->cur_rrn = btree->root;

        btree->num_keys++;

        // Salva pagina
        bnode_save(btree, node);
        free(node);

        return 1;
    } 
    else
    {
        int retorno;
        Bnode* left_page;

        // Podemos usar a estrutura bnode para simplificar a passagem de argumentos para a função
        // de inserir e assim tornar o código mais legível
        Bnode aux, promoted;
        
        aux.cur_rrn = btree->root;
        aux.key[0] = key;
        aux.byte_offset[0] = byte_offset;

        // Recebe o no raiz da subarvore
        retorno = inserir(btree, &aux, &promoted);

        // Caso tenha que promover
        if (retorno)
        {    
            // Cria uma nova pagina
            node = bnode_create();
            
            // Le a pagina da esquerda
            left_page = bnode_read(btree, btree->root);
            
            // Escreve as informacoes da primeira 
            node->key[0] = promoted.key[0];
            node->byte_offset[0] = promoted.byte_offset[0];
            node->next_rrn[0] = btree->root;
            node->next_rrn[1] = promoted.cur_rrn;
            node->num_keys++;
            node->height= left_page->height + 1;
            node->cur_rrn = btree->next_rrn;
            
            // Atualiza as informacoes do cabecalho da subarvore
            btree->root = btree->next_rrn;
            btree->num_keys++;
            btree->max_height++;
            btree->next_rrn++;
            
            // Salva a pagina
            bnode_save(btree, node);

            // Libera as variaveis
            free(node);
            free(left_page);
        
        }
    }
    return 1;
}

// Percorre o arquivo de indice buscando uma chave
// int buscaPagina(int RRN, FILE* index, int chave, int *acessos){
//     PAGINA *page;
//     int POS, ret;

//     // Caso nao exista nada nesta posicao (folha), finaliza a operacao
//     if(RRN == VAZIO) return DEURUIM;

//     // Ler a pagina da posicao atual
//     page = lerPagina(RRN, index);

//     // Conta quantos acessos a pagina foram feitos durante a busca
//     (*acessos)++;

//     // Procurar no arquivo de indices a chave, recebe o sinal que dentro do vetor do no existe a chave
//     if((POS = buscaInterna(page->C, page->n, chave)) == ENCONTROU){
//         // Sabendo que a chave existe nesse vetor, rercorre o este ate achar a chave
//         for(int i = 0; i < page->n; i++){
//             if(page->C[i] == chave){ 
//                 ret = page->Pr[i];
//             }
//         }
//         // Liberea a variavel
//         free(page);
//         // Retorna a referencia da chave
//         return ret;
//     } 
//     else{
//         // Caso nao esteja neste no, recursivamente continua percorrendo
//         ret = buscaPagina(page->P[POS], index, chave, acessos);
//         // Libera informacoes
//         free(page);
//         return ret;
//     } 

//     return -7;
// }

// Recebe um id nascimento e conta quantos acessos a paginas de disco foram feitos durante a busca 
// int buscaArvoreB(FILE* dataset, FILE* indexBtree, int chave){
//     // Le as informacoes da arvore e do arquivo de registro
//     ARVOREB *bTree = lerCabecalhoArvoreB(indexBtree);
//     CABECALHO *header = lerCabecalhoBin(dataset);
//     REGISTRO aux;
//     int acessos = 0;
    
//     // Caso qualquer um dos dois for inconsistente, finaliza a operaca
//     if(header->status == '0' || bTree->status == '0'){
//         free(header);
//         free(bTree);
//         return DEURUIM;
//     }

//     // Percorre o aquivo de registro ate a chave de busca
//     int RRNRegistro = buscaPagina(bTree->noRaiz, indexBtree, chave, &acessos);

//     // Caso tenha sido removido, ou nao inserido, retorna erro e finaliza a operacao
//     if(RRNRegistro == DEURUIM) printf("Registro inexistente.\n");
//     else{
//     // Caso encontrou a chave, imprime as informacoes
//         encontrarRegistroBin(dataset, RRNRegistro, &aux);
//         imprimirRegistro(aux);
//         printf("Quantidade de paginas da arvore-B acessadas: %d\n", acessos);
//     } 

//     // Libera a memoria
//     free(header);
//     free(bTree);
//     return SUCESSO;
// }
