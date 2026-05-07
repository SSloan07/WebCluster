#include "Cluster.h"
#include "RoundRobin.h"
#include <stdlib.h>
#include <string.h>

load_balancer_t* cluster_init() {
    load_balancer_t *lb = malloc(sizeof(load_balancer_t));
    if (!lb) {
        return NULL;
    }

    lb->servers = NULL;      
    lb->server_count = 0;
    lb->current = 0;         
    lb->alive_count = 0;     

    return lb;
}

int cluster_add_server(load_balancer_t *lb, const char *ip, int port) {
    if (!lb || !ip) {
        return -1;
    }

    backend_t *new_servers = realloc(lb->servers, sizeof(backend_t) * (lb->server_count + 1));
    if (!new_servers) {
        return -1; 
    }
    lb->servers = new_servers;

    char *new_ip = strdup(ip);
    if (!new_ip) {
        return -1; 
    }

    backend_t *new_server = &lb->servers[lb->server_count];
    new_server->ip = new_ip;   
    new_server->port = port;
    new_server->is_alive = 0; 

    lb->server_count++;

    return 0;
}

int cluster_remove_server(load_balancer_t *lb, const char *ip, int port) {
    if (!lb || !ip || lb->server_count == 0) {
        return -1;
    }

    int index = -1;
    for (int i = 0; i < lb->server_count; i++) {
        if (lb->servers[i].port == port && strcmp(lb->servers[i].ip, ip) == 0) {
            index = i;
            break;
        }
    }

    if (index == -1){
        return -1; 
    }

    free(lb->servers[index].ip);

    for (int i = index; i < lb->server_count - 1; i++) {
        lb->servers[i] = lb->servers[i + 1];
    }

    lb->server_count--;

    if (lb->server_count == 0) {
        free(lb->servers);
        lb->servers = NULL;
        lb->current = 0;
        return 0;
    }

    backend_t *new_servers = realloc(lb->servers, sizeof(backend_t) * lb->server_count);
    if (new_servers) {
        lb->servers = new_servers;
    }

    if (lb->current >= lb->server_count) {
        lb->current = 0;
    }

    return 0;
}
