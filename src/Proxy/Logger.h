#ifndef LOGGER_H
#define LOGGER_H

#include <stddef.h>

int logger_init(const char *filename);
void logger_close(void);

void logger_request(
    const char *client_ip,
    int client_port,
    const char *backend_ip,
    int backend_port,
    const char *data,
    size_t len
);

void logger_response(
    const char *client_ip,
    int client_port,
    const char *backend_ip,
    int backend_port,
    const char *data,
    size_t len
);

void logger_info(const char *message);
void logger_error(const char *message);

#endif