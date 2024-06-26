#ifndef H_ARVOREB_
#define H_ARVOREB_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_KEYS 5
#define PAGE_SIZE 60

typedef struct {
    unsigned char status;       // Teste para saber se o arquivo esta consistente
    int32_t root;                 // No incial da arvore
    int32_t max_height;              // Altura maxima da arvore (partindo da raiz ate as folhas
    int32_t next_rrn;                // RRN da proxima pagina da arvore B. Sempre que um novo node eh criado, o RRN eh incrementado
    int32_t num_keys;              // Numero total de chaves inseridas no arquivo
    char trash[55];

    FILE *f_pointer; // File pointer para a árvore B
} Btree;

typedef struct {
    int32_t height;      // O nivel dos nos eh feito de baixo para cima, ou seja, a raiz possui o maior nivel e a folha o menor (1)
    int32_t num_keys;          // Numero de chaves presente no node (m = 6, entao n varia de (m/2)-1 = 2 a m -1 = 5)
    int32_t key[MAX_KEYS - 1];     // Chave de busca
    int64_t byte_offset[MAX_KEYS - 1];    // Campo de referencia do registro que a chave guarda, presente no arquivo de dados
    int32_t next_rrn[MAX_KEYS];       // Referencia para a proxima subarvore (caso nao exista recebe -1)
    int32_t cur_rrn;
} Bnode;

// int32_t criarIndiceArvoreB(FILE* bin, FILE* indice);
// int32_t buscarIndice(FILE* indexBtree, int32_ chave);
int32_t btree_insert(Btree *btree, int32_t key, int32_t byte_offset);
Btree* btree_create(char *path);

bool btree_save_header(Btree *btree, char status);

// int32_t buscaArvoreB(FILE* dataset, FILE* indexBtree, int32_ chave);
// int32_t inserirRegistroArvoreB(FILE* bin, FILE* indice);

#endif