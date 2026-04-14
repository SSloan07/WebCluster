#include "../src/Network/socket.h"
#include "../src/Proxy/LoadBalancer.h"

typedef struct {
    net_socket_t *client_sock;
    load_balancer_t *lb;
    // pthread_mutex_t *mutex; // Opcional, si quieres sincronizar el printf
} thread_args_t;
