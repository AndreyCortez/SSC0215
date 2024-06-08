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
    fwrite(reg.data, sizeof(int8_t), tam_data, file);

    // Calcula o numero de caracteres lixo dentro do registro e preenche os mesmo
    int tam_garbage = reg.tam_reg - register_header_size - tam_data;
    while (tam_garbage > 0)
    {
        fputc('$', file);
        tam_garbage--;
    }

    fflush(file);
    return true;
}

Register read_register(int64_t offset, FILE *file)
{
    // offset de retorno do f_pointer quando a função terminar de ser executada
    int64_t ini_offset = ftell(file);

    fseek(file, offset, SEEK_SET);

    Register reg;

    // Lê os dados do proximo registrador
    reg.byte_offset = ftell(file);
    fread(&(reg.removed), sizeof(reg.removed), 1, file);
    fread(&(reg.tam_reg), sizeof(reg.tam_reg), 1, file);
    fread(&(reg.prox_reg), sizeof(reg.prox_reg), 1, file);

    // Aloca dados conforme o necessário
    int data_size = reg.tam_reg - register_header_size;
    reg.data = malloc(data_size);
    fread(reg.data, sizeof(char), data_size, file);

    fseek(file, ini_offset, SEEK_SET);
    return reg;
}

// O unico campo alocado dinamicamente dentro da estrutura de registro é
// o data e por isso precisa ser desalocado ao terminar de usar o registro
void free_register(Register* reg)
{
    if (reg->data != NULL)
        free(reg->data);
}