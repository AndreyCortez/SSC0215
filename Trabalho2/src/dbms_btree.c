#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "dbms_btree.h"
// dentro de cada pagina, as chaves sao ordenadas em ordem crescente
void swap_bytes(void *a, void *b, size_t n) 
{
    void *temp = malloc(n);

    memcpy(temp, a, n);
    memcpy(a, b, n);
    memcpy(b, temp, n);

    free(temp);
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
    node->num_keys = 0;
    node->cur_rrn = -1;

    // Inicializando os campos da arvore
    for(int i = 0; i < MAX_KEYS - 1; i++){
        node->next_rrn[i] = -1;
        node->byte_offset[i] = -1;
        node->key[i] = -1;
    }

    node->next_rrn[MAX_KEYS - 1] = -1;

    return node;
}

// Ler cabecalho de uma arvore ja existente
Btree *btree_acess(char *path){
    FILE *index = fopen(path, "r+b");

    if (index == NULL)
        return NULL;

    Btree *aux = malloc(sizeof(Btree));
    aux->f_pointer = index;
    
    fseek(index, 0, SEEK_SET);
    // Caso haja falha na alocacao, nao insere-se nada no cabecalho
    if(aux == NULL) return aux;
    
    // Leitura de todos os campos
    fread(&(aux->status), sizeof(unsigned char), 1, index);
    
    if (aux->status != '1')
    {
        fclose(index);
        return NULL;
    }

    fread(&(aux->root), sizeof(int), 1, index);
    fread(&(aux->next_rrn), sizeof(int), 1, index);
    fread(&(aux->num_keys), sizeof(int), 1, index);
    fread(&(aux->trash), sizeof(char), PAGE_SIZE - 13, index);

    fflush(index);

    return aux;
}

// Apos alteracores em uma arvore, estas devem ser salvas no seu respectivo arquivo
bool btree_save_header(Btree *btree, char status)
{
    FILE *index = btree->f_pointer;
    rewind(index);
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
    fflush(index);

    return true;
}

// Ler um pagina ja existente
Bnode *bnode_read(Btree* btree, int RRN)
{
    fflush(btree->f_pointer);
    // Utilizaremos uma pagina auxiliar para receber os valores da leitura
    Bnode *node = bnode_create();
    node->cur_rrn = RRN;
    
    // Ajustar o ponteiro para o RRN correto
    fseek(btree->f_pointer, PAGE_SIZE * (1 + RRN), SEEK_SET);
    
    // Ler os campos
    fread(&(node->height), sizeof(int), 1, btree->f_pointer);
    fread(&(node->num_keys), sizeof(int), 1, btree->f_pointer);

    //printf("%d\n", node->num_keys);
    
    for(int i = 0; i < MAX_KEYS - 2; i++)
    {
        fread(&(node->key[i]), sizeof(int), 1, btree->f_pointer);
        fread(&(node->byte_offset[i]), sizeof(int64_t), 1, btree->f_pointer);
        
        // printf("%d ", node->key[i]);
        // printf("%d \n", node->byte_offset[i]);
    }
    // printf("\n");
    
    for(int i = 0; i < MAX_KEYS - 1; i++)
    {
        fread(&(node->next_rrn[i]), sizeof(int), 1, btree->f_pointer);
        // printf("%d ", node->next_rrn[i]);
    }

    // printf("\n=================");
    return node;
}

// Guardar uma pagina
int bnode_save(Btree* btree, Bnode* bnode)
{
    fflush(btree->f_pointer);
    rewind(btree->f_pointer);
    // Ajustar o ponteiro para a posicao correta
    fseek(btree->f_pointer, PAGE_SIZE * (1 + bnode->cur_rrn), SEEK_SET);

    // Escrever todos os campos
    fwrite(&(bnode->height), sizeof(int), 1, btree->f_pointer);
    fwrite(&(bnode->num_keys), sizeof(int), 1, btree->f_pointer);
    
    for(int i = 0; i < MAX_KEYS - 2; i++)
    {
        fwrite(&(bnode->key[i]), sizeof(int), 1, btree->f_pointer);
        fwrite(&(bnode->byte_offset[i]), sizeof(int64_t), 1, btree->f_pointer);
    }
    
    for(int i = 0; i < MAX_KEYS - 1; i++) 
        fwrite(&(bnode->next_rrn[i]), sizeof(int), 1, btree->f_pointer);
    
    return 1;
}

int32_t bnode_insert_pos(Bnode* bnode, int key){
    int *v = bnode->key;
    int tam = bnode->num_keys; 

    for(int i = 0; i < tam; i++)
    {
        if(v[i] >= key)
            return i;
    }
    
    return tam;
}

// Dividir o vetor de chaves
int split(Bnode* page, Bnode* newpage, Bnode *promoted)
{
	newpage->num_keys = 2;
	newpage->key[0] = page->key[2];
	newpage->key[1] = page->key[3];
	newpage->byte_offset[0] = page->byte_offset[2];
	newpage->byte_offset[1] = page->byte_offset[3];
	newpage->next_rrn[0] = page->next_rrn[2];
	newpage->next_rrn[1] = page->next_rrn[3];
	newpage->next_rrn[2] = page->next_rrn[4];
	newpage->height = page->height;

    promoted->key[0] = page->key[1];
    promoted->byte_offset[0] = page->byte_offset[1];
    promoted->cur_rrn = newpage->cur_rrn;

	page->num_keys = 1;
	page->key[1] = -1;
	page->byte_offset[1] = -1;
	page->key[2] = -1;
	page->byte_offset[2] = -1;
	page->key[3] = -1;
	page->byte_offset[3] = -1;
	page->next_rrn[2] = page->next_rrn[3] = page->next_rrn[4] = -1;

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
        pos = bnode_insert_pos(page, inserted->key[0]);

        int32_t cur_rrn = inserted->cur_rrn;

        // Recursivamente verifica se eh possivel guardar a pagina na posicao atual da recursao
        inserted->cur_rrn = page->next_rrn[pos]; 
        retorno = inserir(btree, inserted, promoted);                    
        
        if(!retorno)
        { 
            free(page);
            return false;
        }
        
        // Caso na pagina atual ainda exista espaco para esta chave, sera escrito neste local
       
        // Shifita os itens para frente e os insere, esta em uma ordenacao do vetor bastante eficiente para este caso
        for(int i = (page->num_keys - 1); i >= pos; i--)
        {
            swap_bytes(&(page->key[i]), &(page->key[i+1]), sizeof(int32_t));
            swap_bytes(&(page->byte_offset[i]), &(page->byte_offset[i+1]), sizeof(int64_t));
            swap_bytes(&(page->next_rrn[i+1]), &(page->next_rrn[i+2]), sizeof(int32_t));
        }

        // Escrever as informacoes
        page->key[pos] = promoted->key[0];
        page->byte_offset[pos] = promoted->byte_offset[0];
        page->next_rrn[pos + 1] = promoted->cur_rrn;
        page->num_keys++;
        page->cur_rrn = cur_rrn;

        // for (int i = 0; i < page->num_keys; i++)
        // {
        //     printf("%d", page->key[i]);
        // }
        // printf("\n");
        
        
        // Deu overflow e temos que criar uma nova pagina e apos isso fazer split
        if (page->num_keys >= MAX_KEYS - 1)
        { 
            // Cria pagina
            newpage = bnode_create();
            newpage->cur_rrn = btree->next_rrn;
            
            // Faz overflow
            split(page, newpage, promoted);
            
            // printf("%d %d %d\n", page->next_rrn[0], page->next_rrn[1], page->next_rrn[2]);
            // printf("%d %d %d\n", newpage->next_rrn[0], newpage->next_rrn[1], newpage->next_rrn[2]);

            // Escreve as novas paginas
            bnode_save(btree, page);
            bnode_save(btree, newpage);
            btree->next_rrn++;

            // Libera as alocacoes
            free(page);
            free(newpage);
            return true;
        }
        else
        {
            // printf("%d %d %d\n", page->next_rrn[0], page->next_rrn[1], page->next_rrn[2]);
        	// Salvar pagina     
        	bnode_save(btree, page);
        	btree->num_keys++;  
            free(page);
            return false;
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
        node->num_keys = 1;
        node->height = 0;
        node->cur_rrn = 0;

        btree->num_keys++;



        // Salva pagina
        // printf("oi %d %d %d\n", node->next_rrn[0], node->next_rrn[1], node->next_rrn[2]);
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
            node->num_keys = 1;
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

// Percorre o arquivo de índice buscando uma chave
int btree_search(Btree* btree, int chave, int32_t rrn) {
    // Caso não ache
    if (rrn == -1)
        return -1;

    Bnode *page = bnode_read(btree, rrn);

    int32_t ret = -1;
    int32_t pos = bnode_insert_pos(page, chave);

    if (pos < page->num_keys)
    {
        // Procurar no arquivo de índices a chave
        if (page->key[pos] == chave) 
            ret = page->byte_offset[pos];
        else 
            ret = btree_search(btree, chave, page->next_rrn[pos]);

    } 
    else
    {
        if (page->key[pos - 1] == chave) 
            ret = page->byte_offset[pos];
        else if (page->key[pos - 1] < chave)
            ret = btree_search(btree, chave, page->next_rrn[pos]);
    }

    free(page);
    return ret;
}