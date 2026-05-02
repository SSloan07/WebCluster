#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

#include "../Network/backend.h"

/*This struct is very important because it provides organization  and information about web cluster*/
typedef struct load_balancer {
    backend_t *servers;
    int server_count;
    int current;
    int alive_count;
} load_balancer_t;

#endif