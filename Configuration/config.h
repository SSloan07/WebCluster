#ifndef CONFIG_H
#define CONFIG_H

#define MAX_BACKENDS 32

typedef struct {
    char ip[64];
    int port;
} backend_config_t;

typedef struct {
    int proxy_port;
    int cache_ttl;
    backend_config_t backends[MAX_BACKENDS];
    int backend_count;
} proxy_config_t;

int load_config(const char *filename, proxy_config_t *config);

#endif
