#include "../src/Proxy/Cluster.h"
#include "../src/Proxy/Logger.h"
#include "../src/Proxy/LoadBalancer.h"
#include "../src/Proxy/RoundRobin.h"
#include "../src/Network/socket.h"
#include "../src/Network/tcp.h"
#include "../src/HTTP/HttpParser.h"
#include "../src/HTTP/http_peer/utils/enumToString.h"
#include "../src/cache/Manage_cache.h"
#include "thread_args.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

#define RED   "\033[31m"
#define GREEN "\033[32m"
#define CYAN  "\033[36m"
#define BLUE  "\033[94m"
#define RESET "\033[0m"

/* Lee un archivo cacheado y lo envia completo al cliente. */
static int send_cached_response(net_socket_t *client_sock, const char *file_path) {
    FILE *cache_file;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    int read_error = 0;

    cache_file = fopen(file_path, "rb");
    if (cache_file == NULL) {
        return -1;
    }

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), cache_file)) > 0) {
        if (tcp_send_all(client_sock->fd, buffer, bytes_read) < 0) {
            fclose(cache_file);
            return -1;
        }
    }

    if (ferror(cache_file)) {
        read_error = 1;
    }

    fclose(cache_file);
    return read_error ? -1 : 0;
}

void *manage_client(void *arg) {
    thread_args_t *args = (thread_args_t *) arg;
    net_socket_t *client_sock = args->client_sock;
    load_balancer_t *lb = args->lb;
    pthread_mutex_t *mutex = args->mutex;
    cache_store_t *cache_store = args->cache_store;

    char buffer[BUFFER_SIZE];
    char cache_key[CACHE_KEY_MAX];
    char cache_file_path[CACHE_PATH_MAX];
    net_socket_t *backend_sock = NULL;
    backend_t *target_server = NULL;
    Request request = {0};
    http_parse_result_t parse_result = HTTP_PARSE_ERROR;
    int should_cache_response = 0;
    char *response_buffer = NULL;
    size_t response_size = 0;

    pthread_mutex_lock(mutex);

    printf(CYAN "[CONEXION] Cliente conectado desde %s:%d\n" RESET,
           client_sock->ip_in, client_sock->port_in);

    printf(BLUE "[HEALTHCHECK] Verificando estado de servidores...\n" RESET);
    ping_servers(lb);

    target_server = lb_next_server(lb);

    pthread_mutex_unlock(mutex);

    if (target_server == NULL) {
        printf(RED "[ERROR] No hay backend disponible.\n" RESET);
        logger_error("No hay backend disponible");
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    printf(CYAN "\n[BALANCEADOR] Redirigiendo a -> %s:%d\n" RESET,
           target_server->ip, target_server->port);

    ssize_t bytes_received = tcp_recv(client_sock->fd, buffer, sizeof(buffer));
    if (bytes_received <= 0) {
        printf(RED "[ERROR] No se pudo leer del cliente o se desconecto.\n" RESET);
        logger_error("No se pudo leer del cliente o se desconecto");
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    parse_result = http_parse_request(buffer, (size_t) bytes_received, &request);

    if (parse_result == HTTP_PARSE_OK) {
        printf(CYAN "[HTTP] Metodo: %s\n" RESET, methodToString(request.method));
        printf(CYAN "[HTTP] URI: %s\n" RESET, request.requestURI);
        printf(CYAN "[HTTP] Version: %s\n" RESET, versionToString(request.httpVersion));

        const char *host = http_request_get_header(&request, "Host");
        if (host != NULL) {
            printf(CYAN "[HTTP] Host: %s\n" RESET, host);
        }

        if (!http_request_is_method_supported(&request)) {
            printf(RED "[HTTP] Metodo no soportado por el proyecto.\n" RESET);
        }
    } else if (parse_result == HTTP_PARSE_INCOMPLETE) {
        printf(RED "[HTTP] Peticion HTTP incompleta.\n" RESET);
    } else {
        printf(RED "[HTTP] Error al parsear la peticion HTTP.\n" RESET);
    }

    printf(BLUE "[INFO] Se recibieron %zd bytes del cliente.\n" RESET, bytes_received);

    logger_request(
        client_sock->ip_in,
        client_sock->port_in,
        target_server->ip,
        target_server->port,
        buffer,
        (size_t) bytes_received
    );

    memset(cache_key, 0, sizeof(cache_key));
    memset(cache_file_path, 0, sizeof(cache_file_path));

    int cache_status = http_request_is_cacheable(&request);
    if (cache_status == IS_CACHEABLE) {

        cache_result_t lookup_result;

        printf(GREEN "[CACHE] La peticion es cacheable, verificando cache...\n" RESET);

        if (http_build_cache_key(&request, cache_key, sizeof(cache_key)) == CACHE_SUCCESS) {
            printf(GREEN "[CACHE] Clave de cache construida: %s\n" RESET, cache_key);

            lookup_result = cache_lookup(cache_store, cache_key, cache_file_path, sizeof(cache_file_path));

            if (lookup_result == CACHE_SUCCESS) {
                printf(GREEN "[CACHE] Recurso encontrado en disco: %s\n" RESET, cache_file_path);

                if (send_cached_response(client_sock, cache_file_path) == 0) {
                    printf(GREEN "[CACHE] Respuesta enviada desde cache, sin contactar backend.\n" RESET);
                    logger_info("Respuesta servida desde cache");
                    http_request_free(&request);
                    tcp_close(client_sock);
                    free(args);
                    return NULL;
                }

                printf(RED "[CACHE] No se pudo leer el archivo cacheado, se usara el backend.\n" RESET);
                should_cache_response = 1;
            } else if (lookup_result == CACHE_EXPIRED) {
                printf(BLUE "[CACHE] La entrada existia pero ya expiro.\n" RESET);
                should_cache_response = 1;
            } else if (lookup_result == CACHE_NOT_FOUND) {
                printf(BLUE "[CACHE] No existe entrada en cache para esta clave.\n" RESET);
                should_cache_response = 1;
            } else {
                printf(RED "[CACHE] Error consultando la cache.\n" RESET);
            }
        } else {
            printf(RED "[CACHE] Error al construir la clave de cache.\n" RESET);
        }
    } else {
        printf(BLUE "[CACHE] La peticion no es cacheable.\n" RESET);
    }

    backend_sock = tcp_connect(target_server->ip, target_server->port);
    if (backend_sock == NULL) {
        printf(RED "[ERROR] No se pudo conectar al backend.\n" RESET);
        logger_error("No se pudo conectar al backend seleccionado");
        http_request_free(&request);
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    if (tcp_send_all(backend_sock->fd, buffer, (size_t) bytes_received) < 0) {
        printf(RED "[ERROR] Fallo el envio hacia el backend.\n" RESET);
        logger_error("Fallo el envio hacia el backend");
        http_request_free(&request);
        tcp_close(backend_sock);
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    printf(GREEN "[PROXY] Peticion reenviada al backend.\n" RESET);

    ssize_t bytes_backend;
    while ((bytes_backend = tcp_recv(backend_sock->fd, buffer, sizeof(buffer))) > 0) {
        logger_response(
            client_sock->ip_in,
            client_sock->port_in,
            target_server->ip,
            target_server->port,
            buffer,
            (size_t) bytes_backend
        );

        if (tcp_send_all(client_sock->fd, buffer, (size_t) bytes_backend) < 0) {
            printf(RED "[ERROR] Fallo el reenvio de respuesta al cliente.\n" RESET);
            logger_error("Fallo el reenvio de respuesta al cliente");
            break;
        }

        if (should_cache_response) {
            char *new_buffer = realloc(response_buffer, response_size + (size_t) bytes_backend);
            if (new_buffer == NULL) {
                printf(RED "[CACHE] No se pudo reservar memoria para guardar la respuesta.\n" RESET);
                free(response_buffer);
                response_buffer = NULL;
                response_size = 0;
                should_cache_response = 0;
            } else {
                response_buffer = new_buffer;
                memcpy(response_buffer + response_size, buffer, (size_t) bytes_backend);
                response_size += (size_t) bytes_backend;
            }
        }
    }

    if (bytes_backend < 0) {
        printf(RED "[ERROR] Fallo la lectura desde el backend.\n" RESET);
        logger_error("Fallo la lectura desde el backend");
    } else {
        printf(GREEN "[PROXY] Comunicacion completada correctamente.\n" RESET);
        logger_info("Comunicacion completada correctamente");

        if (should_cache_response && response_buffer != NULL && response_size > 0) {
            if (cache_save(cache_store, cache_key, response_buffer, response_size) == CACHE_SUCCESS) {
                printf(GREEN "[CACHE] Respuesta guardada en disco correctamente.\n" RESET);
            } else {
                printf(RED "[CACHE] No se pudo guardar la respuesta en disco.\n" RESET);
            }
        }
    }

    free(response_buffer);
    http_request_free(&request);
    tcp_close(backend_sock);
    tcp_close(client_sock);
    free(args);

    return NULL;
}
