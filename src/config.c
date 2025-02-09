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

int load_config(const char *config_file, server_config_t *config) {
    // Valores padrão
    config->port = 8080;
    config->max_connections = 10;
    config->buffer_size = 4096;
    
    FILE *f = fopen(config_file, "r");
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
            }
        }
    }
    
    fclose(f);
    return 0;
}