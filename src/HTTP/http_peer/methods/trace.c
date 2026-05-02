#include "trace.h"
#include "../utils/enumToString.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HTTP_Status HTTPTrace(Request *req, HTTP_Response *res) {
    char trace_buffer[4096];
    size_t offset = 0;

    if (req == NULL || res == NULL || req->requestURI == NULL || req->headerList == NULL) {
        return STATUS_400;
    }

    if (req->body != NULL && req->bodyLength > 0) {
        return STATUS_400;
    }

    int written = snprintf(
        trace_buffer + offset,
        sizeof(trace_buffer) - offset,
        "%s %s %s\r\n",
        methodToString(req->method),
        req->requestURI,
        versionToString(req->httpVersion)
    );
    if (written < 0 || (size_t) written >= sizeof(trace_buffer) - offset) {
        return STATUS_500;
    }
    offset += (size_t) written;

    for (size_t i = 0; i < req->headerList->count; i++) {
        const char *header_name = req->headerList->headers[i].name;

        if (header_name == NULL) {
            header_name = headerToString(req->headerList->headers[i].type);
        }

        written = snprintf(
            trace_buffer + offset,
            sizeof(trace_buffer) - offset,
            "%s: %s\r\n",
            header_name,
            req->headerList->headers[i].value
        );
        if (written < 0 || (size_t) written >= sizeof(trace_buffer) - offset) {
            return STATUS_500;
        }
        offset += (size_t) written;
    }

    written = snprintf(trace_buffer + offset, sizeof(trace_buffer) - offset, "\r\n");
    if (written < 0 || (size_t) written >= sizeof(trace_buffer) - offset) {
        return STATUS_500;
    }
    offset += (size_t) written;

    res->content = malloc(offset + 1);
    if (res->content == NULL) {
        return STATUS_500;
    }

    memcpy(res->content, trace_buffer, offset);
    res->content[offset] = '\0';
    res->contentLength = offset;

    char content_length[32];
    snprintf(content_length, sizeof(content_length), "%zu", res->contentLength);

    addResponseHeader(res->headerList, "Content-Type", "message/http");
    addResponseHeader(res->headerList, "Content-Length", content_length);

    return STATUS_200;
}
