#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include "HttpTypes.h"
#include <stddef.h>

http_parse_result_t http_parse_request(
    const char *buffer,
    size_t len,
    http_request_t *request
);

http_parse_result_t http_parse_response(
    const char *buffer,
    size_t len,
    http_response_t *response
);

const char *http_request_get_header(
    const http_request_t *request,
    const char *key
);

const char *http_response_get_header(
    const http_response_t *response,
    const char *key
);

HTTP_Status processRequest(Request *req , HTTP_Response *res );

int http_request_is_method_supported(const http_request_t *request);

int http_request_is_cacheable(const http_request_t *request);

int http_build_cache_key(
    const http_request_t *request,
    char *out,
    size_t out_size
);

void http_request_free(http_request_t *request);
void http_response_free(http_response_t *response);

#endif