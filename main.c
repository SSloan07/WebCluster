#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h> // Para cerrar melo con Ctrl+C
#include "src/Network/tcp.h"

int main() {
    printf("--- INICIANDO SERVIDOR INFINITO (Ctrl+C para salir) ---\n");

    // 1. Crear el servidor una sola vez
    int server_fd = tcp_create_server("127.0.0.1", 8080, 5);
    if (server_fd == -1) return 1;

    printf("Servidor listo en el puerto 8080. Esperando gente...\n");

    // 2. Bucle infinito para atender clientes
    while(1) {
        printf("\nEsperando un nuevo cliente...\n");
        
        net_socket_t *client = tcp_accept(server_fd);
        if (client == NULL) continue; // Si falla uno, seguimos esperando al siguiente

        printf("¡Cliente conectado desde %s:%d!\n", client->ip_in, client->port_in);

        // 3. Lógica de comunicación
        char buffer[1024] = {0};
        ssize_t bytes = tcp_recv(client->fd, buffer, sizeof(buffer) - 1);
        
        if (bytes > 0) {
            printf("Cliente dice: %s\n", buffer);
            const char *msg = "Recibido melo. ¡Siguiente!\n";
            tcp_send_all(client->fd, msg, strlen(msg));
        }

        // 4. MUY IMPORTANTE: Liberar al cliente antes de volver a empezar
        // Si no haces esto, se te llena la RAM (Memory Leak)
        tcp_close(client);
        printf("Cliente despachado con éxito.\n");
    }

    // El código nunca llegará aquí a menos que rompas el while
    close(server_fd);
    return 0;
}
