#include "src/Network/socket.h"
#include "src/Network/tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BACKLOG 10
#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <puerto> [nombre_backend]\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    if (port < 0) {
        printf("Error, puerto invalido\n");
        return -1;
    }

    const char *backend_name = (argc >= 3) ? argv[2] : "backend";

    printf("=== Iniciando %s en puerto %d ===\n", backend_name, port);

    int server_fd = tcp_create_server(NULL, port, BACKLOG);
    if (server_fd < 0) {
        fprintf(stderr, "[ERROR] No se pudo crear el servidor backend en puerto %d\n", port);
        return 1;
    }

    printf("[%s] Escuchando en puerto %d...\n", backend_name, port);

    while (1) {
        printf("\n[%s] Esperando conexión...\n", backend_name);

        net_socket_t *client_sock = tcp_accept(server_fd);
        if (client_sock == NULL) {
            printf("[%s][ERROR] Falló tcp_accept\n", backend_name);
            continue;
        }

        printf("[%s] Conexión desde %s:%d\n",
               backend_name,
               client_sock->ip_in,
               client_sock->port_in);

        char buffer[BUFFER_SIZE];
        ssize_t n = tcp_recv(client_sock->fd, buffer, sizeof(buffer) - 1);

        if (n <= 0) {
            printf("[%s] No se recibieron datos o el cliente cerró la conexión.\n", backend_name);
            tcp_close(client_sock);
            continue;
        }

        buffer[n] = '\0';

        printf("[%s] Recibidos %zd bytes:\n%s\n", backend_name, n, buffer);

        char body[BUFFER_SIZE];
        snprintf(body, sizeof(body),
                 "Hola, soy %s en el puerto %d\n",
                 backend_name, port);

        char response[BUFFER_SIZE];
        int response_len = snprintf(
            response,
            sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: %zu\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            strlen(body),
            body
        );

        if (response_len < 0 || response_len >= (int)sizeof(response)) {
            printf("[%s][ERROR] La respuesta HTTP excede el buffer.\n", backend_name);
            tcp_close(client_sock);
            continue;
        }

        if (tcp_send_all(client_sock->fd, response, (size_t)response_len) < 0) {
            printf("[%s][ERROR] No se pudo enviar la respuesta.\n", backend_name);
        } else {
            printf("[%s] Respuesta HTTP enviada correctamente.\n", backend_name);
        }

        tcp_close(client_sock);
    }

    close(server_fd);
    return 0;
}