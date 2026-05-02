#ifndef ROUND_ROBIN_H
#define ROUND_ROBIN_H 

#include "LoadBalancer.h"

int ping_servers(load_balancer_t *lb);
backend_t* lb_next_server(load_balancer_t *lb);

#endif 