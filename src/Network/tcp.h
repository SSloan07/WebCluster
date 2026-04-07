#ifndef TCP_H
#define TCP_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

int tcp_create_server(int port, int backlog);
int tcp_accept(int server_fd);
int tcp_connect(const char *ip, int port);
ssize_t tcp_send_all(int fd, const void *buf, size_t len);
ssize_t tcp_recv(int fd, void *buf, size_t len);
void tcp_close(int fd);

#endif