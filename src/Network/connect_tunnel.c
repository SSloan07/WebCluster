#include "tcp.h"
#include "../HTTP/HttpParser.h"

#include <stdio.h>
#include <string.h>
#include <sys/select.h>

#define TCP_TUNNEL_BUFFER_SIZE 4096

static int send_connect_response(net_socket_t *client_sock, const char *response_text) {
    return tcp_send_all(client_sock->fd, response_text, strlen(response_text)) < 0 ? -1 : 0;
}

static int tunnel_loop(int client_fd, int destination_fd) {
    fd_set read_fds;
    char buffer[TCP_TUNNEL_BUFFER_SIZE];
    int max_fd = (client_fd > destination_fd) ? client_fd : destination_fd;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(client_fd, &read_fds);
        FD_SET(destination_fd, &read_fds);

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) <= 0) {
            return -1;
        }

        if (FD_ISSET(client_fd, &read_fds)) {
            ssize_t bytes_read = tcp_recv(client_fd, buffer, sizeof(buffer));
            if (bytes_read <= 0) {
                break;
            }

            if (tcp_send_all(destination_fd, buffer, (size_t) bytes_read) < 0) {
                return -1;
            }
        }

        if (FD_ISSET(destination_fd, &read_fds)) {
            ssize_t bytes_read = tcp_recv(destination_fd, buffer, sizeof(buffer));
            if (bytes_read <= 0) {
                break;
            }

            if (tcp_send_all(client_fd, buffer, (size_t) bytes_read) < 0) {
                return -1;
            }
        }
    }

    return 0;
}

int tcp_handle_connect(net_socket_t *client_sock, const char *request_uri) {
    char target_ip[64];
    int target_port;
    net_socket_t *destination_sock;

    if (client_sock == NULL) {
        return -1;
    }

    if (parse_request_connect(request_uri, target_ip, sizeof(target_ip), &target_port) != 0) {
        printf("[CONNECT][ERROR] Invalid target: %s\n", request_uri != NULL ? request_uri : "(null)");
        send_connect_response(
            client_sock,
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n"
        );
        return -1;
    }

    printf("[CONNECT] Target: %s:%d\n", target_ip, target_port);

    destination_sock = tcp_connect(target_ip, target_port);
    if (destination_sock == NULL) {
        printf("[CONNECT][ERROR] Could not connect to target\n");
        send_connect_response(
            client_sock,
            "HTTP/1.1 502 Bad Gateway\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n"
        );
        return -1;
    }

    if (send_connect_response(
            client_sock,
            "HTTP/1.1 200 Connection Established\r\n"
            "\r\n"
        ) < 0) {
        tcp_close(destination_sock);
        return -1;
    }

    printf("[CONNECT] Tunnel established\n");
    tunnel_loop(client_sock->fd, destination_sock->fd);
    printf("[CONNECT] Tunnel closed\n");

    tcp_close(destination_sock);
    return 0;
}
