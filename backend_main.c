#include "src/Network/socket.h"
#include "src/Network/tcp.h"
#include "src/HTTP/HttpParser.h"
#include "src/HTTP/http_peer/utils/enumToString.h"
#include "backend_main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BACKLOG 10
#define BUFFER_SIZE 8192

#define RED   "\033[31m"
#define RESET "\033[0m"
// Mandar respuesta de error al cliente
static int send_text_response(
    int fd,
    int status_code,
    const char *reason_phrase,
    const char *body,
    int send_body
) {
    char headers[1024];
    size_t body_len = (body == NULL) ? 0 : strlen(body);

    int header_len = snprintf(
        headers,
        sizeof(headers),
        "HTTP/1.1 %d %s\r\n"
        "Content-Length: %zu\r\n"
        "Content-Type: text/plain\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code,
        reason_phrase,
        body_len
    );
    if (header_len < 0 || header_len >= (int)sizeof(headers)) {
        return -1;
    }

    if (tcp_send_all(fd, headers, (size_t)header_len) < 0) {
        return -1;
    }

    if (send_body && body_len > 0 && tcp_send_all(fd, body, body_len) < 0) {
        return -1;
    }

    return 0;
}

static int send_http_response(int fd, const HTTP_Response *response, int send_body) {
    char header_buffer[8192];
    size_t offset = 0;

    if (response == NULL || response->headerList == NULL) {
        return -1;
    }
    // Se pasa a string el status y version para construir la respuesta HTTP
    const char *version = versionToString(response->httpVersion);
    const char *status_code = statusToString(response->status);
    const char *reason_phrase = statusToReasonPhrase(response->status);
    // Escibir la línea de estado y los headers
    // 
    int written = snprintf(
        header_buffer + offset, // En que posición escribir
        sizeof(header_buffer) - offset, // Cuanto espacio queda
        "%s %s %s\r\n", // Formato en lo que lo voy a escribir (4 strings y un salto de linea)
        version,  // Lo que voy a escribir
        status_code,
        reason_phrase
    );
    if (written < 0 || (size_t)written >= sizeof(header_buffer) - offset) {
        return -1;
    }
    offset += (size_t)written;

    for (size_t i = 0; i < response->headerList->count; i++) {
        written = snprintf(
            header_buffer + offset,
            sizeof(header_buffer) - offset,
            "%s: %s\r\n",
            response->headerList->headers[i].name,
            response->headerList->headers[i].value
        );
        if (written < 0 || (size_t)written >= sizeof(header_buffer) - offset) {
            return -1;
        }
        offset += (size_t)written;
    }

    written = snprintf(header_buffer + offset, sizeof(header_buffer) - offset, "Connection: close\r\n\r\n");
    if (written < 0 || (size_t)written >= sizeof(header_buffer) - offset) {
        return -1;
    }
    offset += (size_t)written;

    if (tcp_send_all(fd, header_buffer, offset) < 0) {
        return -1;
    }

    if (send_body && response->content != NULL && response->contentLength > 0) {
        if (tcp_send_all(fd, response->content, response->contentLength) < 0) {
            return -1;
        }
    }

    return 0;
}

static void ensure_error_response(HTTP_Response *response) {
    if (response == NULL || response->headerList == NULL) {
        return;
    }

    if (response->status == STATUS_200 || response->status == STATUS_201) {
        return;
    }

    if (response->content == NULL) {
        const char *reason_phrase = statusToReasonPhrase(response->status);
        size_t body_size = strlen(reason_phrase) + 3;
        response->content = malloc(body_size);
        if (response->content != NULL) {
            snprintf(response->content, body_size, "%s\n", reason_phrase);
            response->contentLength = strlen(response->content);
        }
    }

    if (http_response_get_header(response, "Content-Type") == NULL) {
        addResponseHeader(response->headerList, "Content-Type", "text/plain");
    }

    if (http_response_get_header(response, "Content-Length") == NULL) {
        char content_length[32];
        snprintf(content_length, sizeof(content_length), "%zu", response->contentLength);
        addResponseHeader(response->headerList, "Content-Length", content_length);
    }
}

int backend_main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <puerto> <document_root> [nombre_backend]\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Error: puerto inválido\n");
        return 1;
    }

    const char *document_root = argv[2];
    const char *backend_name = (argc >= 4) ? argv[3] : "backend";
    http_set_document_root(document_root);

    printf("=== Iniciando %s en puerto %d ===\n", backend_name, port);
    printf("[%s] DocumentRoot: %s\n", backend_name, document_root);

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

        Request request = {0};
        http_parse_result_t parse_result = http_parse_request(buffer, (size_t)n, &request);

        

        if (parse_result != HTTP_PARSE_OK) {
            printf(RED "[HTTP] Error al parsear la petición HTTP.\n" RESET);
            send_text_response(client_sock->fd, 400, "Bad Request", "400 Bad Request\n", 1);
            tcp_close(client_sock);
            continue;
        }

        printf("[%s] Método: %s | URI: %s | Versión: %s\n",
               backend_name,
               methodToString(request.method),
               request.requestURI,
               versionToString(request.httpVersion));

        if (!http_request_is_method_supported(&request)) {
            printf(RED "[HTTP] Método no soportado por el proyecto.\n" RESET);
            send_text_response(client_sock->fd, 405, "Method Not Allowed", "405 Method Not Allowed\n", 1);
            http_request_free(&request);
            tcp_close(client_sock);
            continue;
        }

        if (request.requestURI == NULL) {
            send_text_response(client_sock->fd, 400, "Bad Request", "400 Bad Request\n", 1);
            http_request_free(&request);
            tcp_close(client_sock);
            continue;
        }

        if (strstr(request.requestURI, "..") != NULL) {
            send_text_response(client_sock->fd, 400, "Bad Request", "400 Bad Request\n", 1);
            http_request_free(&request);
            tcp_close(client_sock);
            continue;
        }

        HTTP_Response response = {0};
        response.headerList = createResponseHeaderList();
        if (response.headerList == NULL) {
            send_text_response(client_sock->fd, 500, "Internal Server Error", "500 Internal Server Error\n", 1);
            http_request_free(&request);
            tcp_close(client_sock);
            continue;
        }

        response.status = processRequest(&request, &response);
        ensure_error_response(&response);
        if (send_http_response(client_sock->fd, &response, request.method != METHOD_HEAD) < 0) {
            printf("[%s][ERROR] No se pudo enviar la respuesta HTTP.\n", backend_name);
            http_response_free(&response);
            http_request_free(&request);
            tcp_close(client_sock);
            continue;
        }

        http_response_free(&response);
        http_request_free(&request);
        tcp_close(client_sock);
    }

    close(server_fd);
    return 0;
}
