#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#define MAX_LINE 256

static int parse_line(char *line, char **key, char **value) {
    char *equals = strchr(line, '=');
    if (!equals) return -1;
    
    *equals = '\0';
    *key = line;
    *value = equals + 1;
    
    // Remove espaços em branco
    char *end = *key + strlen(*key) - 1;
    while (end > *key && (*end == ' ' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    
    // Remove espaços do início do valor
    while (**value == ' ') (*value)++;
    
    // Remove espaços do fim do valor
    end = *value + strlen(*value) - 1;
    while (end > *value && (*end == ' ' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    
    return 0;
}

void init_default_config(server_config_t *config) {
    if (!config) return;

    // Valores padrão básicos do servidor
    config->port = 8080;
    config->max_connections = 10;
    config->buffer_size = 8192;
    
    // Valores padrão adicionais
    config->backlog = 10;
    config->timeout_seconds = 30;
    config->timeout_microseconds = 0;
    config->non_blocking = 0;
    
    // Diretório e logging
    strncpy(config->root_directory, "./www", sizeof(config->root_directory) - 1);
    config->logging_enabled = 1;
    strncpy(config->log_file, "http-server.log", sizeof(config->log_file) - 1);
}

int load_config(server_config_t *config, const char *filename) {
    if (!config || !filename) {
        return -1;
    }

    // Inicializa com valores padrão
    init_default_config(config);
    
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Arquivo de configuração não encontrado. Usando valores padrão.\n");
        return -1;
    }
    
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        // Ignora linhas vazias e comentários
        if (line[0] == '\n' || line[0] == '#') continue;
        
        char *key, *value;
        if (parse_line(line, &key, &value) == 0) {
            if (strcmp(key, "port") == 0) {
                config->port = atoi(value);
            } else if (strcmp(key, "max_connections") == 0) {
                config->max_connections = atoi(value);
            } else if (strcmp(key, "buffer_size") == 0) {
                config->buffer_size = atoi(value);
            } else if (strcmp(key, "backlog") == 0) {
                config->backlog = atoi(value);
            } else if (strcmp(key, "timeout_seconds") == 0) {
                config->timeout_seconds = atoi(value);
            } else if (strcmp(key, "timeout_microseconds") == 0) {
                config->timeout_microseconds = atoi(value);
            } else if (strcmp(key, "non_blocking") == 0) {
                config->non_blocking = atoi(value);
            } else if (strcmp(key, "root_directory") == 0) {
                strncpy(config->root_directory, value, sizeof(config->root_directory) - 1);
            } else if (strcmp(key, "logging_enabled") == 0) {
                config->logging_enabled = atoi(value);
            } else if (strcmp(key, "log_file") == 0) {
                strncpy(config->log_file, value, sizeof(config->log_file) - 1);
            }
        }
    }
    
    fclose(f);
    return validate_config(config);
}

int validate_config(const server_config_t *config) {
    if (!config) return -1;

    // Validação da porta
    if (config->port < 1 || config->port > 65535) {
        fprintf(stderr, "Porta deve estar entre 1 e 65535\n");
        return -1;
    }

    // Validação do número máximo de conexões
    if (config->max_connections < 1 || config->max_connections > 1000) {
        fprintf(stderr, "max_connections deve estar entre 1 e 1000\n");
        return -1;
    }

    // Validação do buffer_size
    if (config->buffer_size < 1024 || config->buffer_size > 65536) {
        fprintf(stderr, "buffer_size deve estar entre 1024 e 65536\n");
        return -1;
    }

    // Validação do backlog
    if (config->backlog < 1 || config->backlog > 128) {
        fprintf(stderr, "backlog deve estar entre 1 e 128\n");
        return -1;
    }

    // Validação dos timeouts
    if (config->timeout_seconds < 0 || config->timeout_microseconds < 0) {
        fprintf(stderr, "valores de timeout não podem ser negativos\n");
        return -1;
    }

    // Validação do diretório raiz
    if (strlen(config->root_directory) == 0) {
        fprintf(stderr, "root_directory não pode estar vazio\n");
        return -1;
    }

    // Validação do arquivo de log quando logging está habilitado
    if (config->logging_enabled && strlen(config->log_file) == 0) {
        fprintf(stderr, "log_file não pode estar vazio quando logging está habilitado\n");
        return -1;
    }

    return 0;
}