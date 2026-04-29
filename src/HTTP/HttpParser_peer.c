#include "HttpParser.h"
#include "http_peer/requestParser.h"
#include "http_peer/utils/enumToString.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void reset_request_state(Request *request) {
    if (request == NULL) {
        return;
    }

    request->headerList = NULL;
    request->method = METHOD_NULL;
    request->requestURI = NULL;
    request->httpVersion = VERSION_NULL;
    request->body = NULL;
    request->bodyLength = 0;
}

static void reset_response_state(HTTP_Response *response) {
    if (response == NULL) {
        return;
    }

    response->status = STATUS_NULL;
    response->httpVersion = VERSION_NULL;
    response->headerList = NULL;
    response->content = NULL;
    response->contentLength = 0;
}

http_parse_result_t http_parse_request(
    const char *buffer,
    size_t len,
    Request *request
) {
    if (buffer == NULL || request == NULL || len == 0) {
        return HTTP_PARSE_ERROR;
    }

    reset_request_state(request);
    request->headerList = createRequestHeaderList();
    if (request->headerList == NULL) {
        return HTTP_PARSE_ERROR;
    }

    size_t position = 0;
    if (parseRequestLine(buffer, len, request, &position) != 0) {
        http_request_free(request);
        return HTTP_PARSE_ERROR;
    }

    if (parseHeaders(buffer, len, request, &position) != 0) {
        http_request_free(request);
        return HTTP_PARSE_ERROR;
    }

    if (request->method == METHOD_POST && parseBody(buffer, len, request, &position) != 0) {
        http_request_free(request);
        return HTTP_PARSE_ERROR;
    }

    return HTTP_PARSE_OK;
}

http_parse_result_t http_parse_response(
    const char *buffer,
    size_t len,
    HTTP_Response *response
) {
    (void)buffer;
    (void)len;
    reset_response_state(response);
    return HTTP_PARSE_ERROR;
}

const char *http_request_get_header(const Request *request, const char *key) {
    if (request == NULL || request->headerList == NULL || key == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < request->headerList->count; i++) {
        const Request_Header *header = &request->headerList->headers[i];
        if (strcmp(headerToString(header->name), key) == 0) {
            return header->value;
        }
    }

    return NULL;
}

const char *http_response_get_header(const HTTP_Response *response, const char *key) {
    if (response == NULL || response->headerList == NULL || key == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < response->headerList->count; i++) {
        const Response_Header *header = &response->headerList->headers[i];
        if (strcmp(header->name, key) == 0) {
            return header->value;
        }
    }

    return NULL;
}

int http_request_is_method_supported(const Request *request) {
    if (request == NULL) {
        return 0;
    }

    return request->method == METHOD_GET ||
           request->method == METHOD_HEAD ||
           request->method == METHOD_POST;
}

int http_request_is_cacheable(const Request *request) {
    if (request == NULL) {
        return 0;
    }

    return request->method == METHOD_GET || request->method == METHOD_HEAD;
}

int http_build_cache_key(const Request *request, char *out, size_t out_size) {
    if (request == NULL || out == NULL || out_size == 0 || request->requestURI == NULL) {
        return -1;
    }

    const char *host = http_request_get_header(request, "Host");
    if (host == NULL) {
        host = "no-host";
    }

    return snprintf(out, out_size, "%s_%s", host, request->requestURI) < (int)out_size ? 0 : -1;
}

void http_request_free(Request *request) {
    if (request == NULL) {
        return;
    }

    free(request->requestURI);
    request->requestURI = NULL;

    if (request->headerList != NULL) {
        freeRequestHeaderList(request->headerList);
        request->headerList = NULL;
    }

    free(request->body);
    request->body = NULL;
    request->bodyLength = 0;
    request->method = METHOD_NULL;
    request->httpVersion = VERSION_NULL;
}

void http_response_free(HTTP_Response *response) {
    if (response == NULL) {
        return;
    }

    if (response->headerList != NULL) {
        freeResponseHeaderList(response->headerList);
        response->headerList = NULL;
    }

    free(response->content);
    response->content = NULL;
    response->contentLength = 0;
    response->status = STATUS_NULL;
    response->httpVersion = VERSION_NULL;
}
