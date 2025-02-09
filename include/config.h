#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int port;
    int max_connections;
    int buffer_size;
} server_config_t;

int load_config(const char *config_file, server_config_t *config);

#endif
