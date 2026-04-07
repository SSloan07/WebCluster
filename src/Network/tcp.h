#ifndef TCP_H
#define TCP_H

#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>

// Server
int tcp_create_server(const char *ip, int port, int backlog);
int tcp_accept(int server_fd, char *ip, int *port);

// Client
int tcp_connect(const char *ip, int port);

// Data transfer
ssize_t tcp_send_all(int fd, const void *buf, size_t len);
ssize_t tcp_recv(int fd, void *buf, size_t len);

// Cleanup
void tcp_close(int fd);

#endif