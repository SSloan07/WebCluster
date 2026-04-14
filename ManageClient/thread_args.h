#include "../src/Network/socket.h"
#include "../src/Proxy/LoadBalancer.h"
#include <pthread.h>

typedef struct {
    net_socket_t *client_sock;
    load_balancer_t *lb;
    pthread_mutex_t *mutex; 
} thread_args_t;
