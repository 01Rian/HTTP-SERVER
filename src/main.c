#include <stdio.h>
#include <stdlib.h>
#include "server.h"
#include "config.h"

int main(void) 
{
    server_config_t config;
    
    if (load_config("config/server_config.conf", &config) != 0) {
        printf("Usando configurações padrão\n");
    }

    printf("Iniciando o servidor na porta %d...\n", config.port);
    printf("Máximo de conexões: %d\n", config.max_connections);
    printf("Tamanho do buffer: %d bytes\n", config.buffer_size);
    
    start_server(config.port, &config);
    
    return EXIT_SUCCESS;
}