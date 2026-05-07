#ifndef MANAGE_CLIENT_UTILS_H
#define MANAGE_CLIENT_UTILS_H

#include "../src/HTTP/HttpParser.h"
#include "../src/Network/socket.h"
#include <stddef.h>

typedef enum {
    OPTIONS_FLOW_CONTINUE = 0,
    OPTIONS_FLOW_FINISH = 1,
    OPTIONS_FLOW_ERROR = -1
} options_flow_result_t;

int send_cached_response(net_socket_t *client_sock, const char *file_path);
int send_simple_http_response(net_socket_t *client_sock, const char *response_text);
options_flow_result_t handle_options_request(
    net_socket_t *client_sock,
    const Request *request,
    Request *mutable_request,
    char **forward_request_buffer,
    const char **request_data_to_send,
    size_t *request_size_to_send
);

int parse_backend_status_code(const char *buffer, size_t len);

#endif
