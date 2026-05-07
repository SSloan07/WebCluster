#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static void trim_whitespace(char *str) {
    if (str == NULL || *str == '\0') {
        return;
    }

    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

int load_config(const char *filename, proxy_config_t *config) {
    if (filename == NULL || config == NULL) {
        return -1;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return -1;
    }

    config->proxy_port = 8080;   // valor por defecto
    config->cache_ttl = 60;      // valor por defecto
    config->backend_count = 0;

    char line[256];

    while (fgets(line, sizeof(line), file) != NULL) {
        trim_whitespace(line);

        // Ignorar líneas vacías o comentarios
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }

        if (strncmp(line, "PORT=", 5) == 0) {
            int port = atoi(line + 5);
            if (port <= 0 || port > 65535) {
                fclose(file);
                return -1;
            }
            config->proxy_port = port;
        }
        else if (strncmp(line, "CACHE_TTL=", 10) == 0) {
            int ttl = atoi(line + 10);
            if (ttl < 0) {
                fclose(file);
                return -1;
            }
            config->cache_ttl = ttl;
        }
        else if (strncmp(line, "BACKEND=", 8) == 0) {
            if (config->backend_count >= MAX_BACKENDS) {
                fclose(file);
                return -1;
            }

            char ip[64];
            int port;

            if (sscanf(line + 8, "%63[^:]:%d", ip, &port) != 2) {
                fclose(file);
                return -1;
            }

            if (port <= 0 || port > 65535) {
                fclose(file);
                return -1;
            }

            strncpy(config->backends[config->backend_count].ip, ip,
                    sizeof(config->backends[config->backend_count].ip) - 1);
            config->backends[config->backend_count].ip[
                sizeof(config->backends[config->backend_count].ip) - 1
            ] = '\0';

            config->backends[config->backend_count].port = port;
            config->backend_count++;
        }
        else {
            // línea desconocida -> error
            fclose(file);
            return -1;
        }
    }

    fclose(file);

    if (config->backend_count == 0) {
        return -1;
    }

    return 0;
}
