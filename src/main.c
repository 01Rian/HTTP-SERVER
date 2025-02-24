#include <stdio.h>
#include <stdlib.h>
#include "server.h"
#include "config.h"

int main(void) 
{
    server_config_t config;
    
    // Inicializa a configuração com valores padrão
    init_default_config(&config);
    
    // Tenta carregar configurações do arquivo
    if (load_config(&config, "config/server_config.conf") != 0) {
        printf("Erro ao carregar configuração. Usando configurações padrão.\n");
    }

    printf("Iniciando o servidor...\n");
    printf("Porta: %d\n", config.port);
    printf("Máximo de conexões: %d\n", config.max_connections);
    printf("Tamanho do buffer: %zu bytes\n", config.buffer_size);
    printf("Backlog: %d\n", config.backlog);
    printf("Diretório raiz: %s\n", config.root_directory);
    printf("Logging %s\n", config.logging_enabled ? "habilitado" : "desabilitado");
    if (config.logging_enabled) {
        printf("Arquivo de log: %s\n", config.log_file);
    }
    
    start_server(config.port, &config);
    
    return EXIT_SUCCESS;
}