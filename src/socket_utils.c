#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "socket_utils.h"
#include "config.h"

int create_server_socket(int port, server_config_t *config)
{
    if (!config) {
        fprintf(stderr, "Configuração inválida\n");
        return -1;
    }

    int sockfd;
    struct sockaddr_in server_addr;

    // Cria o socket do servidor
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erro ao criar o socket do servidor");
        return -1;
    }

    // Configura a opção SO_REUSEADDR
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Erro ao configurar SO_REUSEADDR");
        close(sockfd);
        return -1;
    }

    // Configura timeouts se especificados
    if (config->timeout_seconds > 0 || config->timeout_microseconds > 0) {
        struct timeval tv;
        tv.tv_sec = config->timeout_seconds;
        tv.tv_usec = config->timeout_microseconds;
        
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0 ||
            setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
            perror("Erro ao configurar timeouts");
            close(sockfd);
            return -1;
        }
    }

    // Configura modo não-bloqueante se especificado
    if (config->non_blocking) {
        if (set_socket_non_blocking(sockfd) < 0) {
            close(sockfd);
            return -1;
        }
    }

    // Prepara o endereço do servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Associa o socket ao endereço
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao associar o socket ao endereço");
        close(sockfd);
        return -1;
    }

    // Coloca o socket em modo de escuta
    if (listen(sockfd, config->backlog) < 0) {
        perror("Erro ao escutar as conexões");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int set_socket_non_blocking(int socket_fd)
{
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("Erro ao obter flags do socket");
        return -1;
    }

    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Erro ao configurar socket não-bloqueante");
        return -1;
    }

    return 0;
}

int set_socket_timeout(int socket_fd, int seconds, int microseconds)
{
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = microseconds;

    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0 ||
        setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Erro ao configurar timeouts do socket");
        return -1;
    }

    return 0;
}