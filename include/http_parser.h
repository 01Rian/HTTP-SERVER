#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <stddef.h>

// Função básica para realizar o parsing da linha de requisição HTTP.
// Espera uma linha como "GET /caminho HTTP/1.1"
// Retorna 0 em sucesso e um valor diferente em caso de erro.
int parse_http_request(const char *request, char *method, size_t method_size, 
                      char *path, size_t path_size);

#endif
