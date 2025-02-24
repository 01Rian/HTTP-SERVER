#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include "config.h"

/**
 * @file socket_utils.h
 * @brief Utilitários para manipulação de sockets TCP
 * @details Fornece funções para criação, configuração e manipulação de sockets TCP,
 *          abstraindo detalhes de baixo nível da implementação de sockets.
 */

/**
 * @brief Cria e configura um socket TCP para o servidor
 * @details Realiza as seguintes operações:
 *          1. Cria um novo socket TCP
 *          2. Configura as opções do socket (SO_REUSEADDR)
 *          3. Realiza o bind do socket na porta especificada
 *          4. Coloca o socket em modo de escuta
 *
 * @param port Número da porta em que o socket deve escutar
 * @param config Ponteiro para a estrutura de configuração do servidor
 *
 * @return Em caso de sucesso, retorna o descritor do socket (>= 0)
 *         Em caso de erro, retorna um valor negativo
 *
 * @note O socket é configurado para reutilizar endereços (SO_REUSEADDR)
 * @note O backlog (número máximo de conexões pendentes) é definido na configuração
 *
 * Exemplo de uso:
 * @code
 * server_config_t config;
 * // Inicializa configuração...
 * int socket_fd = create_server_socket(8080, &config);
 * if (socket_fd < 0) {
 *     // Trata erro...
 * }
 * @endcode
 */
int create_server_socket(int port, server_config_t *config);

/**
 * @brief Configura um socket para modo não-bloqueante
 * @details Modifica as flags do socket usando fcntl para habilitar 
 *          o modo não-bloqueante (O_NONBLOCK)
 *
 * @param socket_fd Descritor do socket a ser configurado
 * @return 0 em caso de sucesso, -1 em caso de erro
 *
 * @note Esta função é útil quando se deseja implementar I/O não-bloqueante
 * @note O errno é setado em caso de erro
 */
int set_socket_non_blocking(int socket_fd);

/**
 * @brief Define o timeout para operações de socket
 * @details Configura os timeouts de leitura e escrita do socket usando 
 *          a opção SO_RCVTIMEO e SO_SNDTIMEO
 *
 * @param socket_fd Descritor do socket
 * @param seconds Segundos para timeout
 * @param microseconds Microsegundos para timeout
 * @return 0 em caso de sucesso, -1 em caso de erro
 *
 * @note O timeout total é seconds + microseconds/1000000
 * @note Um valor de 0 para ambos os parâmetros desabilita o timeout
 */
int set_socket_timeout(int socket_fd, int seconds, int microseconds);

#endif // SOCKET_UTILS_H
