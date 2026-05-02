#include "src/Proxy/Cluster.h"
#include "src/Proxy/RoundRobin.h"
#include "src/Proxy/Logger.h"
#include "src/Network/socket.h"
#include "src/Network/tcp.h"
#include "ManageClient/manage_client.h"
#include "ManageClient/thread_args.h"
#include "Configuration/config.h"
#include "src/cache/Manage_cache.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BACKLOG 10

int main() {
    printf("\n=== [PIBL-WS] Iniciando Proxy Inverso + Balanceador ===\n");

    proxy_config_t config;
    cache_store_t cache_store;

    if (load_config("pibl.conf", &config) != 0) {
        printf("[ERROR] No se pudo cargar el archivo de configuracion.\n");
        return 1;
    }

    if (logger_init("pibl.log") != 0) {
        printf("[ERROR] No se pudo inicializar el sistema de logs.\n");
        return 1;
    }

    logger_info("PIBL iniciado correctamente");
    logger_info("Archivo de configuracion cargado correctamente");

    load_balancer_t *lb = cluster_init();
    if (!lb) {
        printf("[ERROR] No se pudo inicializar el balanceador de carga.\n");
        logger_error("No se pudo inicializar el balanceador de carga");
        logger_close();
        return 1;
    }

    for (int i = 0; i < config.backend_count; i++) {
        if (cluster_add_server(lb, config.backends[i].ip, config.backends[i].port) != 0) {
            printf("[ERROR] No se pudo agregar backend %s:%d\n",
                   config.backends[i].ip,
                   config.backends[i].port);
            logger_error("No se pudo agregar un backend desde configuracion");
        }
    }

    printf("[INFO] Se registraron %d servidores en el cluster.\n", lb->server_count);
    printf("[INFO] TTL de cache configurado: %d segundos.\n", config.cache_ttl);

    if (cache_init(&cache_store, ".cache", config.cache_ttl) != CACHE_SUCCESS) {
        printf("[ERROR] No se pudo inicializar la cache.\n");
        logger_error("No se pudo inicializar la cache");
        logger_close();
        return 1;
    }

    if (cache_start_cleaner_thread(&cache_store) != CACHE_SUCCESS) {
        printf("[ERROR] No se pudo iniciar el hilo limpiador de cache.\n");
        logger_error("No se pudo iniciar el hilo limpiador de cache");
        logger_close();
        return 1;
    }

    printf("\n[INFO] Verificando estado inicial de los servidores...\n");
    int alive = ping_servers(lb);
    printf("[INFO] Servidores activos detectados: %d\n\n", alive);

    int server_fd = tcp_create_server(NULL, config.proxy_port, BACKLOG);
    if (server_fd < 0) {
        printf("[ERROR] No se pudo levantar el Proxy en el puerto %d.\n", config.proxy_port);
        logger_error("No se pudo levantar el Proxy");
        logger_close();
        return 1;
    }

    printf("[SISTEMA] Proxy escuchando exitosamente en el puerto %d...\n",
           config.proxy_port);

    pthread_mutex_t lb_mutex = PTHREAD_MUTEX_INITIALIZER;

    while (1) {
        printf("[ESPERA] Esperando una nueva conexion de cliente...\n");

        net_socket_t *client_sock = tcp_accept(server_fd);
        if (client_sock == NULL) {
            printf("[ERROR] Fallo la aceptacion del cliente.\n");
            logger_error("Fallo la aceptacion del cliente");
            continue;
        }

        thread_args_t *args = malloc(sizeof(thread_args_t));
        if (args == NULL) {
            printf("[ERROR] No se pudo reservar memoria para el hilo.\n");
            logger_error("No se pudo reservar memoria para el hilo");
            tcp_close(client_sock);
            continue;
        }

        args->client_sock = client_sock;
        args->lb = lb;
        args->mutex = &lb_mutex;
        args->cache_store = &cache_store;

        pthread_t hilo_para_cliente;
        if (pthread_create(&hilo_para_cliente, NULL, manage_client, args) != 0) {
            printf("[ERROR] No se pudo crear el hilo para el cliente.\n");
            logger_error("No se pudo crear el hilo para el cliente");
            tcp_close(client_sock);
            free(args);
            continue;
        }

        pthread_detach(hilo_para_cliente);
    }

    close(server_fd);
    logger_info("PIBL finalizado");
    logger_close();
    return 0;
}
