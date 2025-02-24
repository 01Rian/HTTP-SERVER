#ifndef CONFIG_H
#define CONFIG_H

/**
 * @file config.h
 * @brief Gerenciamento de configuração do servidor HTTP
 * @details Define estruturas e funções para carregar e gerenciar 
 *          as configurações do servidor HTTP.
 */

/**
 * @brief Estrutura que armazena as configurações do servidor
 * @details Contém todos os parâmetros configuráveis do servidor,
 *          incluindo limites, timeouts e opções de operação.
 */
typedef struct {
    /** @brief Porta em que o servidor irá escutar */
    int port;

    /** @brief Número máximo de conexões simultâneas */
    int max_connections;
    
    /** @brief Tamanho do buffer para recebimento de dados */
    size_t buffer_size;
    
    /** @brief Número máximo de conexões pendentes na fila */
    int backlog;
    
    /** @brief Tempo máximo de espera para operações de socket (em segundos) */
    int timeout_seconds;
    
    /** @brief Tempo máximo de espera para operações de socket (em microsegundos) */
    int timeout_microseconds;
    
    /** @brief Flag que indica se o servidor deve usar modo não-bloqueante */
    int non_blocking;
    
    /** @brief Diretório raiz para servir arquivos estáticos */
    char root_directory[256];
    
    /** @brief Flag que indica se o logging está habilitado */
    int logging_enabled;
    
    /** @brief Caminho para o arquivo de log */
    char log_file[256];
} server_config_t;

/**
 * @brief Carrega a configuração do servidor a partir de um arquivo
 * @details Lê e processa o arquivo de configuração especificado,
 *          preenchendo a estrutura server_config_t com os valores
 *          encontrados. Para parâmetros não especificados no arquivo,
 *          utiliza valores padrão.
 *
 * @param config Ponteiro para a estrutura que receberá as configurações
 * @param filename Caminho para o arquivo de configuração
 * @return 0 em caso de sucesso, valor negativo em caso de erro
 *
 * @note O arquivo de configuração deve estar no formato:
 *       chave = valor
 */
int load_config(server_config_t *config, const char *filename);

/**
 * @brief Inicializa a estrutura de configuração com valores padrão
 * @details Preenche todos os campos da estrutura server_config_t
 *          com valores padrão seguros e razoáveis.
 *
 * @param config Ponteiro para a estrutura a ser inicializada
 */
void init_default_config(server_config_t *config);

/**
 * @brief Valida a configuração atual
 * @details Verifica se todos os parâmetros da configuração
 *          estão dentro de limites aceitáveis e são consistentes.
 *
 * @param config Ponteiro para a configuração a ser validada
 * @return 0 se a configuração é válida, valor negativo caso contrário
 */
int validate_config(const server_config_t *config);

#endif // CONFIG_H
