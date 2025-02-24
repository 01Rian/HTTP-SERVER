#ifndef SERVER_H
#define SERVER_H

#include "config.h"

/**
 * @file server.h
 * @brief Módulo principal do servidor HTTP
 * @details Define as funções principais para inicialização e operação do servidor HTTP.
 *          Implementa o ciclo de vida do servidor e o tratamento de conexões cliente.
 */

/**
 * @brief Inicia o servidor HTTP na porta especificada
 * @details Cria um socket servidor, configura-o para aceitar conexões e inicia
 *          o loop principal que aceita conexões de clientes. Para cada conexão
 *          aceita, cria uma nova thread para processar a requisição.
 *
 * @param port Número da porta em que o servidor irá escutar
 * @param config Ponteiro para a estrutura de configuração do servidor
 *
 * @note Esta função é bloqueante e só retorna em caso de erro fatal
 * @note Cada conexão cliente é tratada em uma thread separada
 *
 * Exemplo de uso:
 * @code
 * server_config_t config;
 * // Inicializa configuração...
 * start_server(8080, &config);
 * @endcode
 */
void start_server(int port, server_config_t *config);

/**
 * @brief Thread que manipula uma conexão cliente
 * @details Esta função é executada em uma thread separada para cada cliente.
 *          Processa a requisição HTTP usando o novo parser, gera e envia a
 *          resposta apropriada.
 *
 * Responsabilidades:
 * - Receber e processar requisições HTTP
 * - Validar a requisição usando o parser
 * - Gerar respostas apropriadas
 * - Gerenciar recursos da conexão
 * - Liberar recursos ao finalizar
 *
 * @param arg Ponteiro para a estrutura client_data_t contendo dados da conexão
 * @return void* Sempre retorna NULL
 *
 * @note Esta função não deve ser chamada diretamente, mas sim através de
 *       pthread_create() no loop principal do servidor
 * @note A thread é configurada como detached, então seus recursos são
 *       automaticamente liberados ao terminar
 */
void* handle_client(void* arg);

#endif // SERVER_H