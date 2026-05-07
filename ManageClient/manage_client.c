#include "../src/Proxy/Cluster.h"
#include "../src/Proxy/Logger.h"
#include "../src/Proxy/LoadBalancer.h"
#include "../src/Proxy/RoundRobin.h"
#include "../src/Network/socket.h"
#include "../src/Network/tcp.h"
#include "../src/HTTP/HttpParser.h"
#include "../src/HTTP/http_peer/utils/enumToString.h"
#include "../src/cache/Manage_cache.h"
#include "manage_client_utils.h"
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
    char *forward_request_buffer = NULL;
    size_t response_size = 0;
    const char *request_data_to_send = buffer;
    size_t request_size_to_send = 0;
    options_flow_result_t options_result = OPTIONS_FLOW_CONTINUE;
    int should_invalidate_cache = 0;
    int backend_status_code = -1;
    int backend_status_parsed = 0;

    printf(CYAN "[CONEXION] Cliente conectado desde %s:%d\n" RESET,
           client_sock->ip_in, client_sock->port_in);

    ssize_t bytes_received = tcp_recv(client_sock->fd, buffer, sizeof(buffer));
    if (bytes_received <= 0) {
        printf(RED "[ERROR] No se pudo leer del cliente o se desconecto.\n" RESET);
        logger_error("No se pudo leer del cliente o se desconecto");
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    parse_result = http_parse_request(buffer, (size_t) bytes_received, &request);

    if (parse_result != HTTP_PARSE_OK) {
        printf(RED "[HTTP] Error al parsear la peticion HTTP.\n" RESET);
        send_simple_http_response(
            client_sock,
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n"
        );
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    printf(CYAN "[HTTP] Metodo: %s\n" RESET, methodToString(request.method));
    printf(CYAN "[HTTP] URI: %s\n" RESET, request.requestURI);
    printf(CYAN "[HTTP] Version: %s\n" RESET, versionToString(request.httpVersion));

    const char *host = http_request_get_header(&request, "Host");
    if (host != NULL) {
        printf(CYAN "[HTTP] Host: %s\n" RESET, host);
    }

    request_size_to_send = (size_t) bytes_received;

    if (!http_request_is_method_supported(&request)) {
        printf(RED "[HTTP] Metodo no soportado por el proyecto.\n" RESET);
        send_simple_http_response(
            client_sock,
            "HTTP/1.1 405 Method Not Allowed\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n"
        );
        http_request_free(&request);
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    if (request.method == METHOD_CONNECT) {
        tcp_handle_connect(client_sock, request.requestURI);
        http_request_free(&request);
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    options_result = handle_options_request(
        client_sock,
        &request,
        &request,
        &forward_request_buffer,
        &request_data_to_send,
        &request_size_to_send
    );

    if (options_result == OPTIONS_FLOW_ERROR) {
        http_request_free(&request);
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    if (options_result == OPTIONS_FLOW_FINISH) {
        http_request_free(&request);
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    pthread_mutex_lock(mutex);

    printf(BLUE "[HEALTHCHECK] Verificando estado de servidores...\n" RESET);
    ping_servers(lb);

    target_server = lb_next_server(lb);

    pthread_mutex_unlock(mutex);

    if (target_server == NULL) {
        printf(RED "[ERROR] No hay backend disponible.\n" RESET);
        logger_error("No hay backend disponible");
        http_request_free(&request);
        free(forward_request_buffer);
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    printf(CYAN "\n[BALANCEADOR] Redirigiendo a -> %s:%d\n" RESET,
           target_server->ip, target_server->port);

    printf(BLUE "[INFO] Se recibieron %zd bytes del cliente.\n" RESET, bytes_received);

    logger_request(
        client_sock->ip_in,
        client_sock->port_in,
        target_server->ip,
        target_server->port,
        request_data_to_send,
        request_size_to_send
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
                    free(forward_request_buffer);
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
        if (request.method == METHOD_PUT || request.method == METHOD_DELETE) {
            should_invalidate_cache = 1;
            printf(BLUE "[CACHE] La peticion es de tipo %s. Se invalidara la cache asociada si la respuesta del backend no es de error.\n" RESET,
                   methodToString(request.method));
        } else {
            printf(BLUE "[CACHE] La peticion no es cacheable segun las reglas definidas.\n" RESET);
        }
    }

    backend_sock = tcp_connect(target_server->ip, target_server->port);
    if (backend_sock == NULL) {
        printf(RED "[ERROR] No se pudo conectar al backend.\n" RESET);
        logger_error("No se pudo conectar al backend seleccionado");
        http_request_free(&request);
        free(forward_request_buffer);
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    if (tcp_send_all(backend_sock->fd, request_data_to_send, request_size_to_send) < 0) {
        printf(RED "[ERROR] Fallo el envio hacia el backend.\n" RESET);
        logger_error("Fallo el envio hacia el backend");
        http_request_free(&request);
        free(forward_request_buffer);
        tcp_close(backend_sock);
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    printf(GREEN "[PROXY] Peticion reenviada al backend.\n" RESET);

    ssize_t bytes_backend;
    while ((bytes_backend = tcp_recv(backend_sock->fd, buffer, sizeof(buffer))) > 0) {
        if (!backend_status_parsed) {
            backend_status_code = parse_backend_status_code(buffer, (size_t) bytes_backend);
            if (backend_status_code >= 100) {
                backend_status_parsed = 1;
                printf(BLUE "[HTTP] Backend respondio con estado %d.\n" RESET, backend_status_code);
            }
        }

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

        if (should_invalidate_cache) {
            if (backend_status_parsed && backend_status_code < 400) {
                Request key_request = request;
                cache_result_t delete_result;

                key_request.method = METHOD_GET;
                if (http_build_cache_key(&key_request, cache_key, sizeof(cache_key)) == CACHE_SUCCESS) {
                    delete_result = cache_delete(cache_store, cache_key);
                    if (delete_result == CACHE_SUCCESS) {
                        printf(GREEN "[CACHE] Entrada GET invalidada correctamente.\n" RESET);
                    } else if (delete_result == CACHE_NOT_FOUND) {
                        printf(BLUE "[CACHE] No habia entrada GET para invalidar.\n" RESET);
                    } else {
                        printf(RED "[CACHE] No se pudo invalidar la entrada GET para la clave: %s\n" RESET, cache_key);
                    }
                } else {
                    printf(RED "[CACHE] Error al construir la clave GET para invalidacion.\n" RESET);
                }

                key_request.method = METHOD_HEAD;
                if (http_build_cache_key(&key_request, cache_key, sizeof(cache_key)) == CACHE_SUCCESS) {
                    delete_result = cache_delete(cache_store, cache_key);
                    if (delete_result == CACHE_SUCCESS) {
                        printf(GREEN "[CACHE] Entrada HEAD invalidada correctamente.\n" RESET);
                    } else if (delete_result == CACHE_NOT_FOUND) {
                        printf(BLUE "[CACHE] No habia entrada HEAD para invalidar.\n" RESET);
                    } else {
                        printf(RED "[CACHE] No se pudo invalidar la entrada HEAD para la clave: %s\n" RESET, cache_key);
                    }
                } else {
                    printf(RED "[CACHE] Error al construir la clave HEAD para invalidacion.\n" RESET);
                }
            } else if (backend_status_parsed) {
                printf(BLUE "[CACHE] No se invalido cache porque el backend respondio con error HTTP %d.\n" RESET,
                       backend_status_code);
            } else {
                printf(BLUE "[CACHE] No se pudo determinar el estado HTTP del backend; no se invalido cache por seguridad.\n" RESET);
            }
        }

        if (should_cache_response && response_buffer != NULL && response_size > 0) {
            if (cache_save(cache_store, cache_key, response_buffer, response_size) == CACHE_SUCCESS) {
                printf(GREEN "[CACHE] Respuesta guardada en disco correctamente.\n" RESET);
            } else {
                printf(RED "[CACHE] No se pudo guardar la respuesta en disco.\n" RESET);
            }
        }
    }

    free(response_buffer);
    free(forward_request_buffer);
    http_request_free(&request);
    tcp_close(backend_sock);
    tcp_close(client_sock);
    free(args);

    return NULL;
}
