#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <stddef.h>

// Códigos de erro do parser
typedef enum {
    HTTP_PARSE_OK = 0,
    HTTP_PARSE_INVALID_REQUEST = -1,
    HTTP_PARSE_MEMORY_ERROR = -2,
    HTTP_PARSE_INCOMPLETE = -3,
    HTTP_PARSE_INVALID_METHOD = -4,
    HTTP_PARSE_INVALID_PATH = -5,
    HTTP_PARSE_INVALID_VERSION = -6,
    HTTP_PARSE_HEADER_TOO_LARGE = -7,
    HTTP_PARSE_TOO_MANY_HEADERS = -8
} http_parse_error_t;

// Estrutura para armazenar um header HTTP
typedef struct {
    char *name;   // Nome do header (ex: "Content-Type")
    char *value;  // Valor do header (ex: "text/html")
} http_header_t;

// Estrutura principal da requisição HTTP
typedef struct {
    char method[16];          // Método HTTP (GET, POST, etc)
    char path[1024];         // Caminho requisitado
    char version[16];        // Versão do protocolo (HTTP/1.1)
    http_header_t *headers;  // Array de headers
    size_t header_count;     // Quantidade atual de headers
    size_t max_headers;      // Quantidade máxima de headers
    char *body;             // Corpo da requisição (se houver)
    size_t body_length;     // Tamanho do corpo
} http_request_t;

/**
 * @brief Inicializa uma estrutura de requisição HTTP
 * @param request Ponteiro para a estrutura a ser inicializada
 * @param max_headers Número máximo de headers suportados
 * @return HTTP_PARSE_OK em caso de sucesso, ou código de erro
 */
int http_request_init(http_request_t *request, size_t max_headers);

/**
 * @brief Libera os recursos alocados por uma requisição HTTP
 * @param request Ponteiro para a requisição
 */
void http_request_cleanup(http_request_t *request);

/**
 * @brief Realiza o parsing de uma requisição HTTP completa
 * @param request Ponteiro para a estrutura que armazenará a requisição parseada
 * @param raw_data Dados brutos da requisição
 * @param length Tamanho dos dados
 * @return HTTP_PARSE_OK em caso de sucesso, ou código de erro
 */
int parse_http_request(http_request_t *request, const char *raw_data, size_t length);

/**
 * @brief Adiciona um header à requisição HTTP
 * @param request Ponteiro para a requisição
 * @param name Nome do header
 * @param value Valor do header
 * @return HTTP_PARSE_OK em caso de sucesso, ou código de erro
 */
int http_request_add_header(http_request_t *request, const char *name, const char *value);

/**
 * @brief Busca um header na requisição HTTP
 * @param request Ponteiro para a requisição
 * @param name Nome do header procurado
 * @return Ponteiro para o header encontrado, ou NULL se não encontrado
 */
const http_header_t* http_request_get_header(const http_request_t *request, const char *name);

/**
 * @brief Define o corpo da requisição HTTP
 * @param request Ponteiro para a requisição
 * @param body Dados do corpo
 * @param length Tamanho dos dados
 * @return HTTP_PARSE_OK em caso de sucesso, ou código de erro
 */
int http_request_set_body(http_request_t *request, const char *body, size_t length);

#endif // HTTP_PARSER_H
