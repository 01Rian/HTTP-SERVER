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

#define MAX_HEADERS 50

// Definir a estrutura para passar dados para a thread
typedef struct {
    int client_socket;
    server_config_t *config;
} client_data_t;

// Função auxiliar para enviar resposta HTTP
static void send_http_response(int client_socket, int status_code, const char* status_text,
                             const char* content_type, const char* body) {
    char response[4096];
    int response_len;

    if (body) {
        response_len = snprintf(response, sizeof(response),
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n"
            "\r\n"
            "%s", status_code, status_text, content_type, strlen(body), body);
    } else {
        response_len = snprintf(response, sizeof(response),
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: 0\r\n"
            "\r\n", status_code, status_text, content_type);
    }

    if (response_len > 0) {
        send(client_socket, response, response_len, 0);
    }
}

void* handle_client(void* arg)
{
    // Recebe a conexão do cliente
    client_data_t* client_data = (client_data_t*)arg;
    int client_socket = client_data->client_socket;
    server_config_t* config = client_data->config;

    // Aloca buffer para receber os dados
    char* buffer = malloc(config->buffer_size);
    if (!buffer) {
        perror("Erro ao alocar buffer");
        close(client_socket);
        free(client_data);
        pthread_exit(NULL);
    }
    memset(buffer, 0, config->buffer_size);

    // Inicializa a estrutura da requisição HTTP
    http_request_t request;
    if (http_request_init(&request, MAX_HEADERS) != HTTP_PARSE_OK) {
        send_http_response(client_socket, 500, "Internal Server Error", 
                         "text/plain", "Erro ao inicializar parser");
        free(buffer);
        free(client_data);
        close(client_socket);
        pthread_exit(NULL);
    }

    // Recebe a requisição do cliente
    ssize_t bytes_received = recv(client_socket, buffer, config->buffer_size - 1, 0);

    if (bytes_received < 0) {
        perror("Erro ao receber dados do cliente");
        http_request_cleanup(&request);
        free(buffer);
        close(client_socket);
        free(client_data);
        pthread_exit(NULL);
    }

    printf("Requisição recebida de tamanho: %zd bytes\n", bytes_received);

    // Parse da requisição HTTP
    int parse_result = parse_http_request(&request, buffer, bytes_received);
    
    if (parse_result != HTTP_PARSE_OK) {
        send_http_response(client_socket, 400, "Bad Request", 
                         "text/plain", "Requisição inválida");
        http_request_cleanup(&request);
        free(buffer);
        close(client_socket);
        free(client_data);
        pthread_exit(NULL);
    }

    printf("Método: %s, Caminho: %s, Versão: %s\n", 
           request.method, request.path, request.version);

    // Processa a requisição
    if (strcmp(request.method, "GET") == 0) {
        const char* body = "<html><body><h1>Olá, Mundo!</h1></body></html>";
        send_http_response(client_socket, 200, "OK", "text/html", body);
    } 
    else if (strcmp(request.method, "POST") == 0) {
        // Verifica se há corpo na requisição
        if (request.body && request.body_length > 0) {
            printf("Corpo da requisição recebido: %zu bytes\n", request.body_length);
            send_http_response(client_socket, 200, "OK", 
                             "text/plain", "Dados recebidos com sucesso");
        } else {
            send_http_response(client_socket, 400, "Bad Request", 
                             "text/plain", "Corpo da requisição vazio");
        }
    }
    else {
        send_http_response(client_socket, 405, "Method Not Allowed", 
                         "text/plain", NULL);
    }

    // Limpa recursos
    http_request_cleanup(&request);
    free(buffer);
    close(client_socket);
    free(client_data);
    pthread_exit(NULL);
}

void start_server(int port, server_config_t *config)
{
    int server_socket = create_server_socket(port, config);

    if (server_socket < 0) {
        fprintf(stderr, "Erro ao criar o socket do servidor\n");
        exit(EXIT_FAILURE);
    }

    printf("Servidor HTTP ouvindo na porta %d\n", port);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

        if (client_socket < 0) {
            perror("Erro ao aceitar a conexão");
            continue;
        }

        printf("Conexão aceita de %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port));

        // Aloca e inicializa a estrutura client_data
        client_data_t *client_data = malloc(sizeof(client_data_t));
        if (!client_data) {
            perror("Erro ao alocar memória");
            close(client_socket);
            continue;
        }
        
        client_data->client_socket = client_socket;
        client_data->config = config;

        pthread_t thread_id;

        if (pthread_create(&thread_id, NULL, handle_client, client_data) != 0) {
            perror("Erro ao criar a thread");
            free(client_data);
            close(client_socket);
            continue;
        }

        pthread_detach(thread_id);
    }

    close(server_socket);
}