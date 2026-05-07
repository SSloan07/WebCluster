#include "backend_main.h"
#include "proxy_main.h"

#include <stdio.h>
#include <stdlib.h>

static int delete_proxy_www_directory(void) {
#ifdef _WIN32
    return system("rmdir /s /q www");
#else
    return system("rm -rf ./www");
#endif
}

static int run_proxy_from_menu(void) {
    int delete_result = delete_proxy_www_directory();

    if (delete_result == 0) {
        printf("[INFO] La carpeta ./www fue eliminada antes de iniciar el proxy.\n");
    } else {
        printf("[INFO] No se pudo eliminar ./www o no existia. Se continuara con el proxy.\n");
    }

    return proxy_main();
}

static int run_backend_from_menu(void) {
    char port_buffer[16];
    char document_root[256];
    char backend_name[128];
    char *argv[4];

    printf("Puerto del backend: ");
    if (scanf("%15s", port_buffer) != 1) {
        printf("[ERROR] No se pudo leer el puerto.\n");
        return 1;
    }

    printf("Document root del backend: ");
    if (scanf("%255s", document_root) != 1) {
        printf("[ERROR] No se pudo leer el document root.\n");
        return 1;
    }

    printf("Nombre del backend: ");
    if (scanf("%127s", backend_name) != 1) {
        printf("[ERROR] No se pudo leer el nombre del backend.\n");
        return 1;
    }

    argv[0] = "backend";
    argv[1] = port_buffer;
    argv[2] = document_root;
    argv[3] = backend_name;

    return backend_main(4, argv);
}

int main(void) {
    int option;

    printf("=== [PIBL-WS] Menu de inicio ===\n");
    printf("1. Ejecutar proxy\n");
    printf("2. Ejecutar backend\n");
    printf("Seleccione una opcion: ");

    if (scanf("%d", &option) != 1) {
        printf("[ERROR] Opcion invalida.\n");
        return 1;
    }

    if (option == 1) {
        return run_proxy_from_menu();
    }

    if (option == 2) {
        return run_backend_from_menu();
    }

    printf("[ERROR] Opcion invalida.\n");
    return 1;
}
