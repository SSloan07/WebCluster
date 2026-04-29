#include "HttpParser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static void trim(char *s) {
    if (!s || !*s) return;

    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;

    if (start != s) {
        memmove(s, start, strlen(start) + 1);
    }

    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[len - 1] = '\0';
        len--;
    }
}

static const char *find_header_value(
    const http_header_t *headers,
    int header_count,
    const char *key
) {
    if (!headers || !key) return NULL;

    for (int i = 0; i < header_count; i++) {
        if (strcasecmp(headers[i].key, key) == 0) {
            return headers[i].value;
        }
    }
    return NULL;
}

http_parse_result_t http_parse_request(
    const char *buffer,
    size_t len,
    http_request_t *request
) {
    if (!buffer || !request || len == 0) {
        return HTTP_PARSE_ERROR;
    }

    memset(request, 0, sizeof(*request));

    char local[16384];
    if (len >= sizeof(local)) {
        return HTTP_PARSE_ERROR;
    }

    memcpy(local, buffer, len);
    local[len] = '\0';

    char *header_end = strstr(local, "\r\n\r\n");
    if (!header_end) {
        return HTTP_PARSE_INCOMPLETE;
    }

    size_t header_len = (size_t)(header_end - local);
    char *body_start = header_end + 4;
    size_t body_len = len - (size_t)(body_start - local);

    char *saveptr = NULL;
    char *line = strtok_r(local, "\r\n", &saveptr);
    if (!line) {
        return HTTP_PARSE_ERROR;
    }

    if (sscanf(line, "%15s %1023s %15s",
               request->method,
               request->request_uri,
               request->http_version) != 3) {
        return HTTP_PARSE_ERROR;
    }

    while ((line = strtok_r(NULL, "\r\n", &saveptr)) != NULL) {
        if (*line == '\0') break;

        char *colon = strchr(line, ':');
        if (!colon) continue;

        *colon = '\0';

        if (request->header_count < HTTP_MAX_HEADERS) {
            strncpy(request->headers[request->header_count].key, line,
                    HTTP_HEADER_KEY_MAX - 1);
            request->headers[request->header_count].key[HTTP_HEADER_KEY_MAX - 1] = '\0';

            strncpy(request->headers[request->header_count].value, colon + 1,
                    HTTP_HEADER_VALUE_MAX - 1);
            request->headers[request->header_count].value[HTTP_HEADER_VALUE_MAX - 1] = '\0';

            trim(request->headers[request->header_count].key);
            trim(request->headers[request->header_count].value);

            request->header_count++;
        }
    }

    if (body_len > 0) {
        request->body = malloc(body_len + 1);
        if (!request->body) {
            return HTTP_PARSE_ERROR;
        }
        memcpy(request->body, body_start, body_len);
        request->body[body_len] = '\0';
        request->body_length = body_len;
    }

    request->is_valid = 1;
    return HTTP_PARSE_OK;
}

http_parse_result_t http_parse_response(
    const char *buffer,
    size_t len,
    http_response_t *response
) {
    if (!buffer || !response || len == 0) {
        return HTTP_PARSE_ERROR;
    }

    memset(response, 0, sizeof(*response));

    char local[16384];
    if (len >= sizeof(local)) {
        return HTTP_PARSE_ERROR;
    }

    memcpy(local, buffer, len);
    local[len] = '\0';

    char *header_end = strstr(local, "\r\n\r\n");
    if (!header_end) {
        return HTTP_PARSE_INCOMPLETE;
    }

    char *body_start = header_end + 4;
    size_t body_len = len - (size_t)(body_start - local);

    char *saveptr = NULL;
    char *line = strtok_r(local, "\r\n", &saveptr);
    if (!line) {
        return HTTP_PARSE_ERROR;
    }

    if (sscanf(line, "%15s %d %63[^\r\n]",
               response->http_version,
               &response->status_code,
               response->reason_phrase) < 2) {
        return HTTP_PARSE_ERROR;
    }

    while ((line = strtok_r(NULL, "\r\n", &saveptr)) != NULL) {
        if (*line == '\0') break;

        char *colon = strchr(line, ':');
        if (!colon) continue;

        *colon = '\0';

        if (response->header_count < HTTP_MAX_HEADERS) {
            strncpy(response->headers[response->header_count].key, line,
                    HTTP_HEADER_KEY_MAX - 1);
            response->headers[response->header_count].key[HTTP_HEADER_KEY_MAX - 1] = '\0';

            strncpy(response->headers[response->header_count].value, colon + 1,
                    HTTP_HEADER_VALUE_MAX - 1);
            response->headers[response->header_count].value[HTTP_HEADER_VALUE_MAX - 1] = '\0';

            trim(response->headers[response->header_count].key);
            trim(response->headers[response->header_count].value);

            response->header_count++;
        }
    }

    if (body_len > 0) {
        response->body = malloc(body_len + 1);
        if (!response->body) {
            return HTTP_PARSE_ERROR;
        }
        memcpy(response->body, body_start, body_len);
        response->body[body_len] = '\0';
        response->body_length = body_len;
    }

    response->is_valid = 1;
    return HTTP_PARSE_OK;
}

const char *http_request_get_header(const http_request_t *request, const char *key) {
    if (!request) return NULL;
    return find_header_value(request->headers, request->header_count, key);
}

const char *http_response_get_header(const http_response_t *response, const char *key) {
    if (!response) return NULL;
    return find_header_value(response->headers, response->header_count, key);
}

int http_request_is_method_supported(const http_request_t *request) {
    if (!request || !request->is_valid) return 0;

    return strcmp(request->method, "GET") == 0 ||
           strcmp(request->method, "HEAD") == 0 ||
           strcmp(request->method, "POST") == 0;
}

int http_request_is_cacheable(const http_request_t *request) {
    if (!request || !request->is_valid) return 0;
    return strcmp(request->method, "GET") == 0 || strcmp(request->method, "HEAD") == 0;
}

int http_build_cache_key(const http_request_t *request, char *out, size_t out_size) {
    if (!request || !out || out_size == 0) return -1;

    const char *host = http_request_get_header(request, "Host");
    if (!host) host = "nohost";

    snprintf(out, out_size, "%s_%s", host, request->request_uri);

    for (size_t i = 0; out[i] != '\0'; i++) {
        if (out[i] == '/' || out[i] == '\\' || out[i] == ':' ||
            out[i] == '?' || out[i] == '&' || out[i] == '=' || out[i] == ' ') {
            out[i] = '_';
        }
    }

    return 0;
}

void http_request_free(http_request_t *request) {
    if (!request) return;
    free(request->body);
    request->body = NULL;
    request->body_length = 0;
    request->is_valid = 0;
}

void http_response_free(http_response_t *response) {
    if (!response) return;
    free(response->body);
    response->body = NULL;
    response->body_length = 0;
    response->is_valid = 0;
}