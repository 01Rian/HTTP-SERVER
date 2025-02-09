#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>
#include "server.h"
#include "socket_utils.h"
#include "http_parser.h"
#include "config.h"

// Definir a estrutura para passar dados para a thread
typedef struct {
    int client_socket;
    server_config_t *config;
} client_data_t;

void* handle_client(void* arg)
{
  // Recebe a conexão do cliente
  client_data_t* client_data = (client_data_t*)arg;
  int client_socket = client_data->client_socket;
  server_config_t* config = client_data->config;

  char buffer[config->buffer_size];
  memset(buffer, 0, config->buffer_size); // Limpa o buffer

  ssize_t bytes_received = recv(client_socket, buffer, config->buffer_size - 1, 0); // Recebe a requisição do cliente

  if (bytes_received < 0) {
    perror("Erro ao receber dados do cliente");
    close(client_socket);
    pthread_exit(NULL);
  }

  printf("Requisição recebida:\n%s\n", buffer);

  char method[8], path[1024];

  if (parse_http_request(buffer, method, sizeof(method), path, sizeof(path)) != 0) {

    const char* bad_request_response = "HTTP/1.1 400 Bad Request\r\n"
                                       "Content-Type: text/html\r\n"
                                       "Content-Length: 0\r\n\r\n";

    send(client_socket, bad_request_response, strlen(bad_request_response), 0);
    close(client_socket);
    pthread_exit(NULL);
  	}
  // Trata requisições GET
  if (strcmp(method, "GET") == 0) {
    const char* body = "<html><body><h1>Olá, Mundo!</h1></body></html>";
    char response[config->buffer_size];

    int response_len = snprintf(response, config->buffer_size, 
        "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %zu\r\n"
            "\r\n"
            "%s", strlen(body), body);
    send(client_socket, response, response_len, 0);
  } else {
    const char* method_not_allowed_response = "HTTP/1.1 405 Method Not Allowed\r\n"
                                          "Content-Type: text/html\r\n"
                                          "Content-Length: 0\r\n\r\n";
    send(client_socket, method_not_allowed_response, strlen(method_not_allowed_response), 0);
  }

  close(client_socket);
  free(client_data); // Mantemos apenas esta liberação de memória
  pthread_exit(NULL);
}

void start_server(int port, server_config_t *config)
{
  int server_socket = create_server_socket(port, config);

  if (server_socket < 0) {
    fprintf(stderr, "Erro ao criar o socket do servidor\n");
    exit(EXIT_FAILURE);
  }

  printf("Servidor ouvindo na porta %d\n", port);

  while (1)
  {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

    if (client_socket < 0) {
      perror("Erro ao aceitar a conexão");
      continue;
    }

    printf("Conexão aceita de %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Aloca e inicializa a estrutura client_data
    client_data_t *client_data = malloc(sizeof(client_data_t));
    if (!client_data) {
      perror("Erro ao alocar memória");
      close(client_socket);
      continue;
    }
    
    client_data->client_socket = client_socket;
    client_data->config = config;

    pthread_t thread_id; // Cria uma thread para lidar com a requisição do cliente

    if (pthread_create(&thread_id, NULL, handle_client, client_data) != 0) {
      perror("Erro ao criar a thread");
      free(client_data);
      close(client_socket);
      continue;
    }

    pthread_detach(thread_id); // Libera a thread para que ela possa ser reutilizada
  }

  close(server_socket);
}