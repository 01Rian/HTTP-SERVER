#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include "socket_utils.h"
#include "config.h"

int create_server_socket(int port, server_config_t *config)
{
  int sockfd; // Define o socket do servidor
  struct sockaddr_in server_addr; // Define a estrutura do endereço do servidor

  sockfd = socket(AF_INET, SOCK_STREAM, 0); // Cria o socket do servidor

  if (sockfd < 0) {
    perror("Erro ao criar o socket do servidor");
    return -1;
  }

  int opt = 1;

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("Erro ao configurar o socket");
    close(sockfd);
    return -1;
  }

  memset(&server_addr, 0, sizeof(server_addr)); // Limpa a estrutura do endereço do servidor
  server_addr.sin_family = AF_INET; // Define a família de endereços (IPv4)
  server_addr.sin_addr.s_addr = INADDR_ANY; // Aceita conexões em todas as interfaces
  server_addr.sin_port = htons(port); // Converte a porta para o formato correto

  if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("Erro ao associar o socket ao endereço");
    close(sockfd);
    return -1;
  }

  if (listen(sockfd, config->max_connections) < 0) {
    perror("Erro ao escutar as conexões");
    close(sockfd);
    return -1;
  }

  return sockfd;
}