#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     
#include <arpa/inet.h>
#include "tcp.h"

int socket_creation(){
    int fd = socket(AF_INET, SOCK_STREAM, 0); // It defines the use of TCP and IPv4 with the object socket from library. 

    if (fd == -1){
        printf("Error creating socket"); 
        return fd; 
    }

    printf("Succesfully socket creation"); 
    return fd; 
}

struct sockaddr_in configure_addr(const char *ip, int port){
    struct sockaddr_in server_addr; 
    server_addr.sin_family = AF_INET; // We use this here, because we gonna use IPv4
    server_addr.sin_port = htons(port); 

    if (ip == NULL){
        server_addr.sin_addr.s_addr = INADDR_ANY; // We define any IP because we didn't got it as parameter
    } else {
        server_addr.sin_addr.s_addr = inet_addr(ip); 
    } 
    return server_addr; 
}

// Server funtions

int tcp_create_server(const char *ip, int port, int backlog){
    int fd = socket_creation(); 

    if (fd == -1){
        printf("Mijo que pasó, esto dió error al crear el socket no puede seguir, arregle eso más bien");
        return  -1; 
    }

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

    sock_info->ip_in = strdup(inet_ntoa(client_addr.sin_addr)); // For managing automaticlly pointers 
    sock_info->port_in = ntohs(client_addr.sin_port); // Remember this is a enum field
    
    sock_info->protocol = PROTO_TCP; 

    // We are defining this because we aren't filling host name 
    sock_info->ip_out = NULL; 
    sock_info->port_out = 0; 

    return sock_info; 
}

// Client functions 

net_socket_t* tcp_connect (const char *ip, int port){
    int fd = socket_creation(); 
    if (fd == -1){
        printf("Hubo error al conectarse al crear el file descriptor del lado del cliente"); 
        return NULL; 
    }

    struct sockaddr_in server_addr = configure_addr(ip,port); 

    if (connect(fd, (struct sockaddr *)&server_addr,sizeof(server_addr))){
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

// Management of memory 

void tcp_close (net_socket_t *sock){
    // In case we couldnd inicialize a socket
    if (sock == NULL){
        return; 
    }

    // This is the comunication channel 
    if(sock->fd >= 0){
        close(sock->fd); 
    }   

    // This is for the fields ip from client and server 

    if(sock->ip_in != NULL){
        free(sock->ip_in); 
    }

    if(sock->ip_out != NULL){
        free(sock->ip_out);
    }

    // Liberate the structure
    free(sock); 

    printf("Socket Liberated. Siuuu"); 

    return; 
    

}

// Transfer Data functions 

/*There is a small detail that has to be explain, the function send() of sys/sockets is the part in charge of sending the Segment 
and as we have seen in class a file greater than PDU has to be send in multiples segments. In this ideas order, send() can 
take one Segment and send it to the client, but it doesn't guarantee that all the segments are sent, this is the reason we have to implement 
the following function 'tcp_send_all()' */

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

