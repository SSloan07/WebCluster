#include "src/Network/socket.h"
#include "src/Network/tcp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define BACKLOG 10
#define BUFFER_SIZE 8192
#define FILE_CHUNK_SIZE 4096

static const char *get_mime_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (ext == NULL) return "application/octet-stream";

    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";

    return "application/octet-stream";
}

static int send_headers(
    int fd,
    int status_code,
    const char *reason_phrase,
    const char *content_type,
    size_t content_length
) {
    char headers[1024];

    int header_len = snprintf(
        headers,
        sizeof(headers),
        "HTTP/1.1 %d %s\r\n"
        "Content-Length: %zu\r\n"
        "Content-Type: %s\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code,
        reason_phrase,
        content_length,
        content_type
    );

    if (header_len < 0 || header_len >= (int)sizeof(headers)) {
        return -1;
    }

    return (tcp_send_all(fd, headers, (size_t)header_len) < 0) ? -1 : 0;
}

static int send_text_response(
    int fd,
    int status_code,
    const char *reason_phrase,
    const char *body,
    int send_body
) {
    if (body == NULL) {
        body = "";
    }

    size_t body_len = strlen(body);

    if (send_headers(fd, status_code, reason_phrase, "text/plain", body_len) < 0) {
        return -1;
    }

    if (send_body && body_len > 0) {
        if (tcp_send_all(fd, body, body_len) < 0) {
            return -1;
        }
    }

    return 0;
}

static int send_file_response(
    int fd,
    int status_code,
    const char *reason_phrase,
    const char *content_type,
    const char *filepath,
    size_t file_size,
    int send_body
) {
    if (send_headers(fd, status_code, reason_phrase, content_type, file_size) < 0) {
        return -1;
    }

    if (!send_body) {
        return 0;
    }

    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL) {
        return -1;
    }

    unsigned char chunk[FILE_CHUNK_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(chunk, 1, sizeof(chunk), fp)) > 0) {
        if (tcp_send_all(fd, chunk, bytes_read) < 0) {
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);
    return 0;
}

static const char *find_body(const char *request) {
    const char *body = strstr(request, "\r\n\r\n");
    if (body == NULL) return NULL;
    return body + 4;
}

static int build_safe_path(
    const char *document_root,
    const char *uri,
    char *out_path,
    size_t out_size
) {
    if (document_root == NULL || uri == NULL || out_path == NULL || out_size == 0) {
        return -1;
    }

    /* bloqueo mínimo contra path traversal */
    if (strstr(uri, "..") != NULL) {
        return -1;
    }

    if (strcmp(uri, "/") == 0) {
        return snprintf(out_path, out_size, "%s/index.html", document_root) < (int)out_size ? 0 : -1;
    }

    return snprintf(out_path, out_size, "%s%s", document_root, uri) < (int)out_size ? 0 : -1;
}

int main(int argc, char *argv[]) {
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

        buffer[n] = '\0';

        printf("[%s] Recibidos %zd bytes:\n%s\n", backend_name, n, buffer);

        char method[16] = {0};
        char uri[1024] = {0};
        char version[16] = {0};

        if (sscanf(buffer, "%15s %1023s %15s", method, uri, version) != 3) {
            printf("[%s] Request mal formada -> 400\n", backend_name);

            send_text_response(
                client_sock->fd,
                400,
                "Bad Request",
                "400 Bad Request\n",
                1
            );

            tcp_close(client_sock);
            continue;
        }

        printf("[%s] Metodo: %s | URI: %s | Version: %s\n",
               backend_name, method, uri, version);

        if (strcmp(version, "HTTP/1.1") != 0) {
            send_text_response(
                client_sock->fd,
                400,
                "Bad Request",
                "400 Bad Request - HTTP Version not supported\n",
                1
            );

            tcp_close(client_sock);
            continue;
        }

        if (strcmp(method, "GET") != 0 &&
            strcmp(method, "HEAD") != 0 &&
            strcmp(method, "POST") != 0) {
            send_text_response(
                client_sock->fd,
                400,
                "Bad Request",
                "400 Bad Request - Unsupported Method\n",
                1
            );

            tcp_close(client_sock);
            continue;
        }

        if (strcmp(method, "POST") == 0) {
            /* paso intermedio: POST mínimo */
            const char *body_start = find_body(buffer);

            char response_body[BUFFER_SIZE];
            if (body_start != NULL && *body_start != '\0') {
                snprintf(response_body, sizeof(response_body),
                         "POST recibido por %s en puerto %d\nCuerpo:\n%s\n",
                         backend_name, port, body_start);
            } else {
                snprintf(response_body, sizeof(response_body),
                         "POST recibido por %s en puerto %d\nSin cuerpo o cuerpo vacio\n",
                         backend_name, port);
            }

            send_text_response(
                client_sock->fd,
                200,
                "OK",
                response_body,
                1
            );

            tcp_close(client_sock);
            continue;
        }

        /* GET / HEAD */
        char filepath[2048];
        if (build_safe_path(document_root, uri, filepath, sizeof(filepath)) != 0) {
            send_text_response(
                client_sock->fd,
                400,
                "Bad Request",
                "400 Bad Request - Invalid Path\n",
                strcmp(method, "HEAD") == 0 ? 0 : 1
            );

            tcp_close(client_sock);
            continue;
        }

        struct stat st;
        if (stat(filepath, &st) != 0 || !S_ISREG(st.st_mode)) {
            send_text_response(
                client_sock->fd,
                404,
                "Not Found",
                "404 Page/File Not Found\n",
                strcmp(method, "HEAD") == 0 ? 0 : 1
            );

            tcp_close(client_sock);
            continue;
        }

        const char *content_type = get_mime_type(filepath);
        int send_body = (strcmp(method, "HEAD") == 0) ? 0 : 1;

        if (send_file_response(
                client_sock->fd,
                200,
                "OK",
                content_type,
                filepath,
                (size_t)st.st_size,
                send_body
            ) < 0) {
            printf("[%s][ERROR] No se pudo enviar el archivo: %s\n", backend_name, filepath);
        } else {
            printf("[%s] Recurso servido correctamente: %s\n", backend_name, filepath);
        }

        tcp_close(client_sock);
    }

    close(server_fd);
    return 0;
}