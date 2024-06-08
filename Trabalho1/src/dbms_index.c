#include "dbms_index.h"

void index_free(Index *index)
{
    for (int i = 0; i < index->num_reg; i++)
    {
        free(index->key[i]);
        index->key[i] = NULL;
    }

    free(index->key);
    free(index->byte_offset);

    index->key = NULL;
    index->byte_offset = NULL;
}