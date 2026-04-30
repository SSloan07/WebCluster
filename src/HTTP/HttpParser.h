#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <stddef.h>
#include "structs/request.h"
#include "structs/response.h"

typedef enum {
    HTTP_PARSE_OK = 0,
    HTTP_PARSE_INCOMPLETE = 1,
    HTTP_PARSE_ERROR = -1
} http_parse_result_t;

http_parse_result_t http_parse_request(
    const char *buffer,
    size_t len,
    Request *request
);

http_parse_result_t http_parse_response(
    const char *buffer,
    size_t len,
    HTTP_Response *response
);

const char *http_request_get_header(
    const Request *request,
    const char *key
);

const char *http_response_get_header(
    const HTTP_Response *response,
    const char *key
);

HTTP_Status processRequest(Request *req, HTTP_Response *res);

void http_request_free(Request *request);
void http_response_free(HTTP_Response *response);

#endif
