#include "RoundRobin.h"
#include "LoadBalancer.h"
#include "../Network/socket.h"
#include <stddef.h>

int ping_servers(load_balancer_t *lb) {
    int alive_count = 0;

    for (int i = 0; i < lb->server_count; i++) {

        net_socket_t *test = tcp_connect(lb->servers[i].ip,
                                         lb->servers[i].port);

        if (test != NULL) {
            lb->servers[i].is_alive = 1;
            alive_count++;

            printf("[OK] %s:%d\n",
                   lb->servers[i].ip,
                   lb->servers[i].port);

            tcp_close(test);

        } else {
            lb->servers[i].is_alive = 0;

            printf("[DOWN] %s:%d\n",
                   lb->servers[i].ip,
                   lb->servers[i].port);
        }
    }

    return alive_count;
}
backend_t* lb_next_server(load_balancer_t *lb) {
    if (lb == NULL || lb->server_count == 0) {
        return NULL; // Case we don't have a load balancer initialize or we don't have server associated to the load_balancer
    }

    int attempts = 0;

    while (attempts < lb->server_count) {

        backend_t *server = &lb->servers[lb->current];
        lb->current = (lb->current + 1) % lb->server_count;

        if (server->is_alive) {
            return server;
        }

        attempts++;
    }

    return NULL; // Case we don't have any servers Up 
}