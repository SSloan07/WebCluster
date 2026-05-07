#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "tcp.h"

int socket_creation(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1){
        printf("Error creating socket");
        return fd;
    }

    printf("Succesfully socket creation");
    return fd;
}

struct sockaddr_in configure_addr(const char *ip, int port){
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (ip == NULL){
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        server_addr.sin_addr.s_addr = inet_addr(ip);
    }
    return server_addr;
}

int tcp_create_server(const char *ip, int port, int backlog){
    int fd = socket_creation();

    struct sockaddr_in server_addr = configure_addr(ip, port);

    if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed. Ojo a pesar de que si se creó el socket, no se pudo vincular a la red real ");
        close(fd);
        return -1;
    }

    if (listen(fd, backlog) < 0) {
        perror("Listen failed");
        close(fd);
        return -1;
    }

    return fd;
}

net_socket_t* tcp_accept(int fd_server) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = accept(fd_server, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
        printf("Mijo, hubo errores creando el socket del cliente\n");
        return NULL;
    }

    net_socket_t *sock_info = malloc(sizeof(net_socket_t));
    if (sock_info == NULL) {
        close(client_fd);
        return NULL;
    }

    sock_info->fd = client_fd;
    sock_info->ip_in = strdup(inet_ntoa(client_addr.sin_addr));
    sock_info->port_in = ntohs(client_addr.sin_port);
    sock_info->protocol = PROTO_TCP;
    sock_info->ip_out = NULL;
    sock_info->port_out = 0;

    return sock_info;
}

net_socket_t* tcp_connect (const char *ip, int port){
    int fd = socket_creation();
    if (fd == -1){
        printf("Hubo error al conectarse al crear el file descriptor del lado del cliente");
        return NULL;
    }

    struct sockaddr_in server_addr = configure_addr(ip,port);

    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr))){
        printf("Error al conectarse al server del lado del cliente");
        close(fd);
        return NULL;
    }

    net_socket_t *sock_info = malloc(sizeof(net_socket_t));

    if (sock_info == NULL){
        printf("No se inicializó la estructura de socket");
        close(fd);
        return NULL;
    }

    sock_info->fd = fd;
    sock_info->ip_out = strdup(ip);
    sock_info->port_out = port;
    sock_info->protocol = PROTO_TCP;
    sock_info->ip_in = NULL;
    sock_info->port_in = 0;

    printf("Muy berraco, se conectó al server");

    return sock_info;
}

void tcp_close (net_socket_t *sock){
    if (sock == NULL){
        return;
    }

    if(sock->fd >= 0){
        close(sock->fd);
    }

    if(sock->ip_in != NULL){
        free(sock->ip_in);
    }

    if(sock->ip_out != NULL){
        free(sock->ip_out);
    }

    free(sock);

    printf("Socket Liberated. Siuuu");
}

ssize_t tcp_send_all(int fd, const void *buf, size_t len){
    size_t total_sent = 0;
    const char *p = (const char *) buf;
    int number_of_segments = 0;

    while (total_sent < len){
        ssize_t sent = send(fd,p+total_sent,len - total_sent, 0);
        if (sent == -1){
            printf ("Hubo un error enviando un segmento");
            return -1;
        }

        total_sent += sent;
        number_of_segments += 1;
    }
    printf("Total bytes sent: %zu en %d segmentos.\n", total_sent, number_of_segments);
    return (ssize_t)total_sent;
}

ssize_t tcp_recv(int fd, void *buf, size_t len) {
    ssize_t bytes_read = recv(fd, buf, len, 0);

    if (bytes_read == -1) {
        perror("Ni siquiera llegó el archivo");
        return -1;
    }

    if (bytes_read == 0) {
        printf("El otro lado cerró la conexión\n");
        return 0;
    }

    printf("Se recibieron %zd bytes.\n", bytes_read);
    return (ssize_t)bytes_read;
}
