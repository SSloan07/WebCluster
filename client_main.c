#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "src/Network/tcp.h"

int main() {
    printf("--- INICIANDO CLIENTE DE PRUEBAS ---\n");

    // 1. Conectarse al servidor (IP y Puerto deben coincidir con el servidor)
    net_socket_t *conn = tcp_connect("127.0.0.1", 8080);
    
    if (conn == NULL) {
        printf("No se pudo conectar. ¿Está el servidor prendido?\n");
        return 1;
    }

    // 2. Enviar un mensaje usando tu función pro tcp_send_all
    const char *mensaje = "Hola servidor, soy el cliente berraco!";
    printf("Enviando mensaje...\n");
    tcp_send_all(conn->fd, mensaje, strlen(mensaje));

    // 3. Recibir la respuesta del servidor
    char buffer[1024] = {0};
    ssize_t bytes = tcp_recv(conn->fd, buffer, sizeof(buffer) - 1);
    
    if (bytes > 0) {
        printf("Respuesta del servidor: %s\n", buffer);
    }

    // 4. Limpiar memoria y cerrar socket con tu función
    tcp_close(conn);

    return 0;
}
