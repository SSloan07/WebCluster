#ifndef SOCKET_H
#define SOCKET_H


// Lets define the particle "in" as information releated with source machine and "out" as information releated with target machine

typedef enum {
    PROTO_TCP, 
    PROTO_UDP 
} protocol_t; 

// Represents a network connection (proxy context)

typedef struct net_socket {
    int fd; 

    char *ip_in;   // "255.255.255.255"
    int port_in;

    char *ip_out;
    int port_out;

    int id;
    protocol_t protocol; 
} net_socket_t;


#endif