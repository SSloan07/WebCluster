#include "../src/Network/socket.h"
#include "../src/Proxy/LoadBalancer.h"
#include "../src/cache/cacheTypes.h"
#include <pthread.h>

typedef struct {
    net_socket_t *client_sock;
    load_balancer_t *lb;
    pthread_mutex_t *mutex;
    cache_store_t *cache_store;
} thread_args_t;
