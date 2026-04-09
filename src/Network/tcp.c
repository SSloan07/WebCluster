
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
    sock_info->port_in = ntohs(client_addr.sin_port); // Remeber this is a enum field
    
    sock_info->protocol = PROTO_TCP; 

    // We are defining this because we aren't filling host name 
    sock_info->ip_out = NULL; 
    sock_info->port_out = 0; 

    return sock_info; 
}
