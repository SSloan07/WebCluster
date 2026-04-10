#ifndef CLUSTER_H
#define CLUSTER_H

#include "LoadBalancer.h"

load_balancer_t* cluster_init();
int cluster_add_server(load_balancer_t *lb, const char *ip, int port);
int cluster_remove_server(load_balancer_t *lb, const char *ip, int port);

#endif  