#include "HttpParser.h"
#include "HttpParser.h"
#include "http_peer/requestParser.h"
#include "http_peer/utils/enumToString.h"
#include <string.h>
#include <stdlib.h>
/*
 * Aquí se adapta el parser del compañero a la interfaz del proyecto.
 * Las firmas NO cambian.
 */
// Se realiza la adaptación de la función de mi compañero con el adaptador de manage_client.c
http_parse_result_t http_parse_request(
    const char *buffer,
    size_t len,
    http_request_t *request
) {
    // 1. Se invoca la función parseRequestLine para obtener un RequestLine a partir de la primera línea del request
    // 2. traducir su salida a http_request_t
    // 3. retornar HTTP_PARSE_OK / INCOMPLETE / ERROR
    if (buffer == NULL || request == NULL || len == 0) {
        return HTTP_PARSE_ERROR;
    }

    const char *line_end = strstr(buffer, "\r\n");

    if (line_end == NULL) {
        return HTTP_PARSE_INCOMPLETE;
    }

    size_t request_line_len = (size_t)(line_end - buffer) + 2;
    RequestLine parsed_request;
    memset(&parsed_request, 0, sizeof(RequestLine));
    memset(request, 0, sizeof(http_request_t));

    int result = parseRequestLine(buffer, request_line_len, &parsed_request);

    if (result != 0) {
        if (parsed_request.requestURI != NULL) {
            free(parsed_request.requestURI);
        }
        return HTTP_PARSE_ERROR;
    }

     strncpy(request->method, methodToString(parsed_request.method), HTTP_METHOD_MAX - 1);
    request->method[HTTP_METHOD_MAX - 1] = '\0';

    strncpy(request->request_uri, parsed_request.requestURI, HTTP_URI_MAX - 1);
    request->request_uri[HTTP_URI_MAX - 1] = '\0';

    strncpy(request->http_version, versionToString(parsed_request.httpVersion), HTTP_VERSION_MAX - 1);
    request->http_version[HTTP_VERSION_MAX - 1] = '\0';

    request->header_count = 0;
    request->body = NULL;
    request->body_length = 0;
    request->is_valid = 1;

    free(parsed_request.requestURI);

    return HTTP_PARSE_OK;
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