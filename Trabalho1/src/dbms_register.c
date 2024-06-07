#include "dbms_register.h"

bool write_register(Register reg, FILE *file, char* format)
{
    fseek(file, reg.byte_offset, SEEK_SET);

    // Escreve o registro
    fwrite(&(reg.removed), sizeof(reg.removed), 1, file);
    fwrite(&(reg.tam_reg), sizeof(reg.tam_reg), 1, file);
    fwrite(&(reg.prox_reg), sizeof(reg.prox_reg), 1, file);

    // Calcula o tamanho do campo de dados que não é informação lixo
    int tam_data = size_of_row(reg.data, format);

    // Quando vamos anexar um registro no final do arquivo o tamanho
    // do seu campo de dados é equivalente ao seu tamanho menos o tamanho de seu cabeçalho
    fwrite(reg.data, sizeof(char), tam_data, file);

    // Calcula o numero de caracteres lixo dentro do registro e preenche os mesmo
    int tam_garbage = reg.tam_reg - register_header_size - tam_data;
    while (tam_garbage > 0)
    {
        fputc('$', file);
    }

    fflush(file);
    return true;
}