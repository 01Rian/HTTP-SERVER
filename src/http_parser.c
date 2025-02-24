#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "http_parser.h"

// Funções auxiliares internas
static int parse_request_line(http_request_t *request, const char *data, size_t length, size_t *offset);
static int parse_headers(http_request_t *request, const char *data, size_t length, size_t *offset);
static int is_valid_method(const char *method);
static int is_valid_path_char(char c);
static void skip_whitespace(const char *data, size_t length, size_t *offset);

int http_request_init(http_request_t *request, size_t max_headers) {
    if (!request || max_headers == 0) {
        return HTTP_PARSE_INVALID_REQUEST;
    }

    // Inicializa todos os campos com zeros
    memset(request, 0, sizeof(http_request_t));
    
    // Aloca espaço para os headers
    request->headers = calloc(max_headers, sizeof(http_header_t));
    if (!request->headers) {
        return HTTP_PARSE_MEMORY_ERROR;
    }

    request->max_headers = max_headers;
    request->header_count = 0;
    request->body = NULL;
    request->body_length = 0;

    return HTTP_PARSE_OK;
}

void http_request_cleanup(http_request_t *request) {
    if (!request) {
        return;
    }

    // Libera headers
    if (request->headers) {
        for (size_t i = 0; i < request->header_count; i++) {
            free(request->headers[i].name);
            free(request->headers[i].value);
        }
        free(request->headers);
    }

    // Libera corpo se existir
    if (request->body) {
        free(request->body);
    }

    // Zera a estrutura
    memset(request, 0, sizeof(http_request_t));
}

int parse_http_request(http_request_t *request, const char *raw_data, size_t length) {
    if (!request || !raw_data || length == 0) {
        return HTTP_PARSE_INVALID_REQUEST;
    }

    size_t offset = 0;
    int result;

    // Parse da linha de requisição
    result = parse_request_line(request, raw_data, length, &offset);
    if (result != HTTP_PARSE_OK) {
        return result;
    }

    // Parse dos headers
    result = parse_headers(request, raw_data, length, &offset);
    if (result != HTTP_PARSE_OK) {
        return result;
    }

    // Verifica se há corpo na requisição
    const http_header_t *content_length_header = http_request_get_header(request, "Content-Length");
    if (content_length_header) {
        size_t content_length = atol(content_length_header->value);
        if (content_length > 0 && offset + content_length <= length) {
            // Copia o corpo
            return http_request_set_body(request, raw_data + offset, content_length);
        }
    }

    return HTTP_PARSE_OK;
}

int http_request_add_header(http_request_t *request, const char *name, const char *value) {
    if (!request || !name || !value) {
        return HTTP_PARSE_INVALID_REQUEST;
    }

    if (request->header_count >= request->max_headers) {
        return HTTP_PARSE_TOO_MANY_HEADERS;
    }

    // Aloca e copia o nome do header
    request->headers[request->header_count].name = strdup(name);
    if (!request->headers[request->header_count].name) {
        return HTTP_PARSE_MEMORY_ERROR;
    }

    // Aloca e copia o valor do header
    request->headers[request->header_count].value = strdup(value);
    if (!request->headers[request->header_count].value) {
        free(request->headers[request->header_count].name);
        return HTTP_PARSE_MEMORY_ERROR;
    }

    request->header_count++;
    return HTTP_PARSE_OK;
}

const http_header_t* http_request_get_header(const http_request_t *request, const char *name) {
    if (!request || !name) {
        return NULL;
    }

    for (size_t i = 0; i < request->header_count; i++) {
        if (strcasecmp(request->headers[i].name, name) == 0) {
            return &request->headers[i];
        }
    }

    return NULL;
}

int http_request_set_body(http_request_t *request, const char *body, size_t length) {
    if (!request || (!body && length > 0)) {
        return HTTP_PARSE_INVALID_REQUEST;
    }

    // Libera corpo anterior se existir
    if (request->body) {
        free(request->body);
        request->body = NULL;
        request->body_length = 0;
    }

    if (length > 0) {
        request->body = malloc(length);
        if (!request->body) {
            return HTTP_PARSE_MEMORY_ERROR;
        }

        memcpy(request->body, body, length);
        request->body_length = length;
    }

    return HTTP_PARSE_OK;
}

// Implementação das funções auxiliares internas

static int parse_request_line(http_request_t *request, const char *data, size_t length, size_t *offset) {
    char buffer[1024];
    size_t i = 0;

    // Parse do método
    while (*offset < length && i < sizeof(buffer) - 1 && !isspace(data[*offset])) {
        buffer[i++] = data[(*offset)++];
    }
    buffer[i] = '\0';

    if (!is_valid_method(buffer)) {
        return HTTP_PARSE_INVALID_METHOD;
    }
    // Copia o método para a estrutura
    strncpy(request->method, buffer, sizeof(request->method) - 1);
    request->method[sizeof(request->method) - 1] = '\0'; // Garante que a string seja terminada em nulo

    skip_whitespace(data, length, offset);

    // Parse do path
    i = 0;
    while (*offset < length && i < sizeof(buffer) - 1 && !isspace(data[*offset])) {
        if (!is_valid_path_char(data[*offset])) {
            return HTTP_PARSE_INVALID_PATH;
        }
        buffer[i++] = data[(*offset)++];
    }
    buffer[i] = '\0';
    // Copia o path para a estrutura
    strncpy(request->path, buffer, sizeof(request->path) - 1);
    request->path[sizeof(request->path) - 1] = '\0'; // Garante que a string seja terminada em nulo

    skip_whitespace(data, length, offset);

    // Parse da versão HTTP
    i = 0;
    while (*offset < length && i < sizeof(buffer) - 1 && data[*offset] != '\r' && data[*offset] != '\n') {
        buffer[i++] = data[(*offset)++];
    }
    buffer[i] = '\0';

    if (strncmp(buffer, "HTTP/", 5) != 0) {
        return HTTP_PARSE_INVALID_VERSION;
    }
    strncpy(request->version, buffer, sizeof(request->version) - 1);
    request->version[sizeof(request->version) - 1] = '\0'; // Garante que a string seja terminada em nulo

    // Pula CRLF
    if (*offset + 1 >= length || data[*offset] != '\r' || data[*offset + 1] != '\n') {
        return HTTP_PARSE_INCOMPLETE;
    }
    *offset += 2;

    return HTTP_PARSE_OK;
}

static int parse_headers(http_request_t *request, const char *data, size_t length, size_t *offset) {
    char name[256];
    char value[1024];
    
    while (*offset < length) {
        // Verifica se chegamos ao fim dos headers (linha vazia)
        if (data[*offset] == '\r' && *offset + 1 < length && data[*offset + 1] == '\n') {
            *offset += 2;
            return HTTP_PARSE_OK;
        }

        // Parse do nome do header
        size_t i = 0;
        while (*offset < length && i < sizeof(name) - 1 && data[*offset] != ':') {
            name[i++] = data[(*offset)++];
        }
        name[i] = '\0';

        if (*offset >= length || data[*offset] != ':') {
            return HTTP_PARSE_INCOMPLETE;
        }
        (*offset)++; // Pula o ':'

        // Pula espaços antes do valor
        skip_whitespace(data, length, offset);

        // Parse do valor do header
        i = 0;
        while (*offset < length && i < sizeof(value) - 1 && data[*offset] != '\r') {
            value[i++] = data[(*offset)++];
        }
        value[i] = '\0';

        // Verifica CRLF
        if (*offset + 1 >= length || data[*offset] != '\r' || data[*offset + 1] != '\n') {
            return HTTP_PARSE_INCOMPLETE;
        }
        *offset += 2;

        // Adiciona o header
        int result = http_request_add_header(request, name, value);
        if (result != HTTP_PARSE_OK) {
            return result;
        }
    }

    return HTTP_PARSE_INCOMPLETE;
}

static int is_valid_method(const char *method) {
    static const char *valid_methods[] = {
        "GET", "POST", "HEAD", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH", NULL
    };

    for (const char **m = valid_methods; *m; m++) {
        if (strcmp(method, *m) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_valid_path_char(char c) {
    // Caracteres válidos em uma URL (simplificado)
    return isalnum(c) || c == '/' || c == '.' || c == '-' || c == '_' || c == '~' || c == '%' || 
           c == '+' || c == '=' || c == '&' || c == '?' || c == '#';
}

static void skip_whitespace(const char *data, size_t length, size_t *offset) {
    while (*offset < length && isspace(data[*offset])) {
        (*offset)++;
    }
}