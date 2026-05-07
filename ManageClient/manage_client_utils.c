#include "manage_client_utils.h"
#include "../src/HTTP/http_peer/utils/enumToString.h"
#include "../src/Network/tcp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MANAGE_CLIENT_UTILS_BUFFER_SIZE 4096

int send_cached_response(net_socket_t *client_sock, const char *file_path) {
    FILE *cache_file;
    char buffer[MANAGE_CLIENT_UTILS_BUFFER_SIZE];
    size_t bytes_read;
    int read_error = 0;

    if (client_sock == NULL || file_path == NULL) {
        return -1;
    }

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

int send_simple_http_response(net_socket_t *client_sock, const char *response_text) {
    if (client_sock == NULL || response_text == NULL) {
        return -1;
    }

    return tcp_send_all(client_sock->fd, response_text, strlen(response_text)) < 0 ? -1 : 0;
}

static int handle_options_locally(net_socket_t *client_sock, const Request *request) {
    const char *version;
    char response_buffer[512];
    int written;

    if (client_sock == NULL || request == NULL) {
        return -1;
    }

    version = versionToString(request->httpVersion);
    written = snprintf(
        response_buffer,
        sizeof(response_buffer),
        "%s 200 OK\r\n"
        "Allow: GET, HEAD, POST, PUT, DELETE, TRACE, CONNECT, OPTIONS\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n",
        version
    );

    if (written < 0 || written >= (int) sizeof(response_buffer)) {
        return -1;
    }

    return tcp_send_all(client_sock->fd, response_buffer, (size_t) written) < 0 ? -1 : 0;
}

options_flow_result_t handle_options_request(
    net_socket_t *client_sock,
    const Request *request,
    Request *mutable_request,
    char **forward_request_buffer,
    const char **request_data_to_send,
    size_t *request_size_to_send
) {
    const char *max_forwards_header;
    char *endptr;
    long parsed_value;

    if (client_sock == NULL || request == NULL || mutable_request == NULL ||
        forward_request_buffer == NULL || request_data_to_send == NULL ||
        request_size_to_send == NULL) {
        return OPTIONS_FLOW_ERROR;
    }

    if (request->method != METHOD_OPTIONS) {
        return OPTIONS_FLOW_CONTINUE;
    }

    max_forwards_header = http_request_get_header(request, "Max-Forwards");
    if (max_forwards_header == NULL) {
        return OPTIONS_FLOW_CONTINUE;
    }

    parsed_value = strtol(max_forwards_header, &endptr, 10);
    if (*endptr != '\0' || parsed_value < 0) {
        send_simple_http_response(
            client_sock,
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n"
        );
        return OPTIONS_FLOW_ERROR;
    }

    if (parsed_value == 0) {
        if (handle_options_locally(client_sock, request) != 0) {
            return OPTIONS_FLOW_ERROR;
        }
        return OPTIONS_FLOW_FINISH;
    }

    if (http_request_decrement_max_forwards(mutable_request) != 0) {
        send_simple_http_response(
            client_sock,
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n"
        );
        return OPTIONS_FLOW_ERROR;
    }

    *forward_request_buffer = http_request_to_raw(mutable_request, request_size_to_send);
    if (*forward_request_buffer == NULL) {
        return OPTIONS_FLOW_ERROR;
    }

    *request_data_to_send = *forward_request_buffer;
    return OPTIONS_FLOW_CONTINUE;
}

int parse_backend_status_code(const char *buffer, size_t len) {
    char status_line[128];
    size_t i;
    int status_code;

    if (buffer == NULL || len == 0) {
        return -1;
    }

    for (i = 0; i < len && i + 1 < sizeof(status_line); i++) {
        if (buffer[i] == '\r' || buffer[i] == '\n') {
            break;
        }
        status_line[i] = buffer[i];
    }
    status_line[i] = '\0';

    if (sscanf(status_line, "HTTP/%*s %d", &status_code) != 1) {
        return -1;
    }

    return status_code;
}