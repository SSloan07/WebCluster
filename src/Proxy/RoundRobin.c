#include "RoundRobin.h"
#include "LoadBalancer.h"
#include "../Network/socket.h"
#include "../Network/tcp.h"
#include <stddef.h>
#include <stdio.h>

int ping_servers(load_balancer_t *lb) {
    if (!lb || lb->server_count == 0) {
        return 0;
    }

    int alive_count = 0;

    for (int i = 0; i < lb->server_count; i++) {
        net_socket_t *test = tcp_connect(lb->servers[i].ip, lb->servers[i].port);

        if (test != NULL) {
            lb->servers[i].is_alive = 1;
            alive_count++;

            printf("[OK] %s:%d\n", lb->servers[i].ip,lb->servers[i].port);

            tcp_close(test);
        } else {
            lb->servers[i].is_alive = 0;

            printf("[DOWN] %s:%d\n", lb->servers[i].ip, lb->servers[i].port);
        }
    }

    lb->alive_count = alive_count;

    return alive_count;
}

backend_t* lb_next_server(load_balancer_t *lb) {

    if (lb == NULL || lb->server_count == 0 || lb->servers == NULL) {
        return NULL; 
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

    return NULL; 
}
