#include "HttpParser.h"

/*
 * Aquí se adapta el parser del compañero a la interfaz del proyecto.
 * Las firmas NO cambian.
 */

http_parse_result_t http_parse_request(
    const char *buffer,
    size_t len,
    http_request_t *request
) {
    // 1. invocar parser del compañero
    // 2. traducir su salida a http_request_t
    // 3. retornar HTTP_PARSE_OK / INCOMPLETE / ERROR
    return HTTP_PARSE_ERROR;
}

http_parse_result_t http_parse_response(
    const char *buffer,
    size_t len,
    http_response_t *response
) {
    return HTTP_PARSE_ERROR;
}

const char *http_request_get_header(const http_request_t *request, const char *key) {
    // misma lógica
    return NULL;
}

const char *http_response_get_header(const http_response_t *response, const char *key) {
    return NULL;
}

int http_request_is_method_supported(const http_request_t *request) {
    return 0;
}

int http_request_is_cacheable(const http_request_t *request) {
    return 0;
}

int http_build_cache_key(const http_request_t *request, char *out, size_t out_size) {
    return -1;
}

void http_request_free(http_request_t *request) {
    (void)request;
}

void http_response_free(http_response_t *response) {
    (void)response;
}