#ifndef TCP_H
#define TCP_H

#include <stddef.h>
#include <netinet/in.h> 
#include <arpa/inet.h>  
#include <sys/types.h>
#include <sys/socket.h>
#include "socket.h" // This is our interface for managing Proxy 

// Server
int tcp_create_server(const char *ip, int port, int backlog);
net_socket_t* tcp_accept(int server_fd);

// Server Auxiliary functions
int socket_creation();
struct sockaddr_in configure_addr(const char *ip, int port); 

// Client
net_socket_t* tcp_connect(const char *ip, int port);

// Data transfer
ssize_t tcp_send_all(int fd, const void *buf, size_t len);
ssize_t tcp_recv(int fd, void *buf, size_t len);

// Cleanup
void tcp_close(net_socket_t *sock);

#endif