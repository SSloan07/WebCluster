#include "Logger.h"
#include <stdio.h>
#include <time.h>
#include <pthread.h>



static FILE *log_file = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static void make_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

static void print_data(FILE *out, const char *data, size_t len) {
    if (!data || len == 0) {
        fprintf(out, "(vacío)\n");
        return;
    }

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)data[i];

        if (c == '\r') {
            fprintf(out, "\\r");
        } else if (c == '\n') {
            fprintf(out, "\\n\n");
        } else if (c >= 32 && c <= 126) {
            fputc(c, out);
        } else {
            fprintf(out, "\\x%02X", c);
        }
    }

    fprintf(out, "\n");
}

int logger_init(const char *filename) {
    if (!filename) return -1;

    log_file = fopen(filename, "a");
    if (!log_file) {
        perror("Error abriendo archivo de log");
        return -1;
    }

    return 0;
}

void logger_close(void) {
    
    pthread_mutex_lock(&log_mutex);

    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }

    pthread_mutex_unlock(&log_mutex);
    pthread_mutex_destroy(&log_mutex);
}

void logger_info(const char *message) {
    char ts[32];
    make_timestamp(ts, sizeof(ts));

    pthread_mutex_lock(&log_mutex);

    printf("[%s] [INFO] %s\n", ts, message);
    fflush(stdout);

    if (log_file) {
        fprintf(log_file, "[%s] [INFO] %s\n", ts, message);
        fflush(log_file);
    }

    pthread_mutex_unlock(&log_mutex);
}

void logger_error(const char *message) {
    char ts[32];
    make_timestamp(ts, sizeof(ts));

    pthread_mutex_lock(&log_mutex);

    fprintf(stderr, "[%s] [ERROR] %s\n", ts, message);
    fflush(stderr);

    if (log_file) {
        fprintf(log_file, "[%s] [ERROR] %s\n", ts, message);
        fflush(log_file);
    }

    pthread_mutex_unlock(&log_mutex);
}

void logger_request( const char *client_ip, int client_port, const char *backend_ip, int backend_port, const char *data, size_t len ) {

    char ts[32];
    make_timestamp(ts, sizeof(ts));

    pthread_mutex_lock(&log_mutex);

    printf("\n[%s] ===== REQUEST =====\n", ts);
    printf("Cliente: %s:%d\n", client_ip ? client_ip : "unknown", client_port);
    printf("Backend: %s:%d\n", backend_ip ? backend_ip : "unknown", backend_port);
    printf("Contenido:\n");
    print_data(stdout, data, len);
    printf("=======================\n");
    fflush(stdout);

    if (log_file) {
        fprintf(log_file, "\n[%s] ===== REQUEST =====\n", ts);
        fprintf(log_file, "Cliente: %s:%d\n", client_ip ? client_ip : "unknown", client_port);
        fprintf(log_file, "Backend: %s:%d\n", backend_ip ? backend_ip : "unknown", backend_port);
        fprintf(log_file, "Contenido:\n");
        print_data(log_file, data, len);
        fprintf(log_file, "=======================\n");
        fflush(log_file);
    }

    pthread_mutex_unlock(&log_mutex);
}

void logger_response(const char *client_ip, int client_port, const char *backend_ip, int backend_port, const char *data, size_t len ) {

    char ts[32];
    make_timestamp(ts, sizeof(ts));

    pthread_mutex_lock(&log_mutex);

    printf("\n[%s] ===== RESPONSE =====\n", ts);
    printf("Cliente: %s:%d\n", client_ip ? client_ip : "unknown", client_port);
    printf("Backend: %s:%d\n", backend_ip ? backend_ip : "unknown", backend_port);
    printf("Contenido:\n");
    print_data(stdout, data, len);
    printf("========================\n");
    fflush(stdout);

    if (log_file) {
        fprintf(log_file, "\n[%s] ===== RESPONSE =====\n", ts);
        fprintf(log_file, "Cliente: %s:%d\n", client_ip ? client_ip : "unknown", client_port);
        fprintf(log_file, "Backend: %s:%d\n", backend_ip ? backend_ip : "unknown", backend_port);
        fprintf(log_file, "Contenido:\n");
        print_data(log_file, data, len);
        fprintf(log_file, "========================\n");
        fflush(log_file);
    }

    pthread_mutex_unlock(&log_mutex);
}