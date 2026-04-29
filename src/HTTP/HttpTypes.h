#ifndef HTTP_TYPES_H
#define HTTP_TYPES_H

#include <stddef.h>

// Incluir estructuras del compañero (Request, Response, etc.)
#include "structs/request.h"
#include "structs/response.h"

#define HTTP_METHOD_MAX        16
#define HTTP_URI_MAX         1024
#define HTTP_VERSION_MAX       16
#define HTTP_REASON_MAX        64

#define HTTP_HEADER_KEY_MAX    64
#define HTTP_HEADER_VALUE_MAX 256
#define HTTP_MAX_HEADERS       64
#define MAX_URI_LENGTH 2048
typedef enum {
    HTTP_PARSE_OK = 0,
    HTTP_PARSE_INCOMPLETE = 1,
    HTTP_PARSE_ERROR = -1
} http_parse_result_t;

typedef struct {
    char key[HTTP_HEADER_KEY_MAX];
    char value[HTTP_HEADER_VALUE_MAX];
} http_header_t;

typedef struct {
    char method[HTTP_METHOD_MAX];        // GET | HEAD | POST
    char request_uri[HTTP_URI_MAX];      // /index.html
    char http_version[HTTP_VERSION_MAX]; // HTTP/1.1

    http_header_t headers[HTTP_MAX_HEADERS];
    int header_count;

    char *body;
    size_t body_length;

    int is_valid;
} http_request_t;

typedef struct {
    char http_version[HTTP_VERSION_MAX]; // HTTP/1.1
    int status_code;                     // 200, 400, 404
    char reason_phrase[HTTP_REASON_MAX]; // OK, Bad Request, Not Found

    http_header_t headers[HTTP_MAX_HEADERS];
    int header_count;

    char *body;
    size_t body_length;

    int is_valid;
} http_response_t;

#endif