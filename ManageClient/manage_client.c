#include "../src/Proxy/Cluster.h"
#include "../src/Proxy/RoundRobin.h"
#include "../src/Proxy/Logger.h"
#include "../src/Proxy/LoadBalancer.h"
#include "../src/Network/socket.h"
#include "../src/Network/tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "thread_args.h"

#define PORT 8080
#define BACKLOG 10
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

    char buffer[BUFFER_SIZE];
    net_socket_t *backend_sock = NULL;
    backend_t *target_server = NULL;

    pthread_mutex_lock(mutex);

    printf(CYAN "[CONEXIÓN] Cliente conectado desde %s:%d\n" RESET,
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
        printf(RED "[ERROR] No se pudo leer del cliente o se desconectó.\n" RESET);
        logger_error("No se pudo leer del cliente o se desconectó");
        tcp_close(client_sock);
        free(args);
        return NULL;
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

    backend_sock = tcp_connect(target_server->ip, target_server->port);
    if (backend_sock == NULL) {
        printf(RED "[ERROR] No se pudo conectar al backend.\n" RESET);
        logger_error("No se pudo conectar al backend seleccionado");
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    if (tcp_send_all(backend_sock->fd, buffer, bytes_received) < 0) {
        printf(RED "[ERROR] Falló el envío hacia el backend.\n" RESET);
        logger_error("Falló el envío hacia el backend");
        tcp_close(backend_sock);
        tcp_close(client_sock);
        free(args);
        return NULL;
    }

    printf(GREEN "[PROXY] Petición reenviada al backend.\n" RESET);

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

        if (tcp_send_all(client_sock->fd, buffer, bytes_backend) < 0) {
            printf(RED "[ERROR] Falló el reenvío de respuesta al cliente.\n" RESET);
            logger_error("Falló el reenvío de respuesta al cliente");
            break;
        }
    }

    if (bytes_backend < 0) {
        printf(RED "[ERROR] Falló la lectura desde el backend.\n" RESET);
        logger_error("Falló la lectura desde el backend");
    } else {
        printf(GREEN "[PROXY] Comunicación completada correctamente.\n" RESET);
        logger_info("Comunicación completada correctamente");
    }

    tcp_close(backend_sock);
    tcp_close(client_sock);
    free(args);

    return NULL;
}