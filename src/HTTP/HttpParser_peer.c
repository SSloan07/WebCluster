#include "HttpParser.h"
#include "http_peer/requestParser.h"
#include "http_peer/utils/enumToString.h"

#include <arpa/inet.h>
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

    if ((request->method == METHOD_POST || request->method == METHOD_PUT) &&
        parseBody(buffer, len, request, &position) != 0) {
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

        if (header->name != NULL && strcasecmp(header->name, key) == 0) {
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
           request->method == METHOD_POST ||
           request->method == METHOD_PUT ||
           request->method == METHOD_DELETE ||
           request->method == METHOD_TRACE ||
           request->method == METHOD_CONNECT ||
           request->method == METHOD_OPTIONS;
}

int http_request_decrement_max_forwards(Request *request) {
    Request_Header *header;
    char *endptr;
    long parsed_value;
    char new_value[32];
    char *value_copy;

    if (request == NULL || request->headerList == NULL) {
        return -1;
    }

    header = searchHeader(request->headerList, HEADER_MAX_FORWARDS);
    if (header == NULL || header->value == NULL) {
        return -1;
    }

    parsed_value = strtol(header->value, &endptr, 10);
    if (*endptr != '\0' || parsed_value <= 0) {
        return -1;
    }

    snprintf(new_value, sizeof(new_value), "%ld", parsed_value - 1);
    value_copy = strdup(new_value);
    if (value_copy == NULL) {
        return -1;
    }

    free(header->value);
    header->value = value_copy;
    return 0;
}

char *http_request_to_raw(const Request *request, size_t *out_len) {
    size_t total_size = 0;
    char *raw_request;
    size_t offset = 0;
    int written;

    if (request == NULL || request->requestURI == NULL || request->headerList == NULL || out_len == NULL) {
        return NULL;
    }

    total_size += strlen(methodToString(request->method)) + 1;
    total_size += strlen(request->requestURI) + 1;
    total_size += strlen(versionToString(request->httpVersion)) + 2;

    for (size_t i = 0; i < request->headerList->count; i++) {
        const char *header_name = request->headerList->headers[i].name;

        if (header_name == NULL) {
            header_name = headerToString(request->headerList->headers[i].type);
        }

        total_size += strlen(header_name) + 2;
        total_size += strlen(request->headerList->headers[i].value) + 2;
    }

    total_size += 2;
    total_size += request->bodyLength;
    total_size += 1;

    raw_request = malloc(total_size);
    if (raw_request == NULL) {
        return NULL;
    }

    written = snprintf(
        raw_request + offset,
        total_size - offset,
        "%s %s %s\r\n",
        methodToString(request->method),
        request->requestURI,
        versionToString(request->httpVersion)
    );
    if (written < 0 || (size_t) written >= total_size - offset) {
        free(raw_request);
        return NULL;
    }
    offset += (size_t) written;

    for (size_t i = 0; i < request->headerList->count; i++) {
        const char *header_name = request->headerList->headers[i].name;

        if (header_name == NULL) {
            header_name = headerToString(request->headerList->headers[i].type);
        }

        written = snprintf(
            raw_request + offset,
            total_size - offset,
            "%s: %s\r\n",
            header_name,
            request->headerList->headers[i].value
        );
        if (written < 0 || (size_t) written >= total_size - offset) {
            free(raw_request);
            return NULL;
        }
        offset += (size_t) written;
    }

    written = snprintf(raw_request + offset, total_size - offset, "\r\n");
    if (written < 0 || (size_t) written >= total_size - offset) {
        free(raw_request);
        return NULL;
    }
    offset += (size_t) written;

    if (request->body != NULL && request->bodyLength > 0) {
        memcpy(raw_request + offset, request->body, request->bodyLength);
        offset += request->bodyLength;
    }

    raw_request[offset] = '\0';
    *out_len = offset;
    return raw_request;
}

int parse_request_connect(
    const char *request_uri,
    char *ip_out,
    size_t ip_out_size,
    int *port_out
) {
    const char *colon;
    size_t ip_length;
    char *endptr;
    long parsed_port;
    struct in_addr ipv4_addr;

    if (request_uri == NULL || ip_out == NULL || port_out == NULL || ip_out_size == 0) {
        return -1;
    }

    colon = strchr(request_uri, ':');
    if (colon == NULL) {
        return -1;
    }

    ip_length = (size_t) (colon - request_uri);
    if (ip_length == 0 || ip_length >= ip_out_size) {
        return -1;
    }

    memcpy(ip_out, request_uri, ip_length);
    ip_out[ip_length] = '\0';

    if (*(colon + 1) == '\0') {
        return -1;
    }

    parsed_port = strtol(colon + 1, &endptr, 10);
    if (*endptr != '\0' || parsed_port < 1 || parsed_port > 65535) {
        return -1;
    }

    if (inet_pton(AF_INET, ip_out, &ipv4_addr) != 1) { //internet presentation to network address, valida que la ip sea correcta
        return -1;
    }

    *port_out = (int) parsed_port;
    return 0;
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
