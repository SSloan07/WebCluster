
#include "tcp.h"

int tcp_create_server(const char *ip, int port, int backlog){
    // Lets apply the step to create a server in this function 
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    return fd; 
}
