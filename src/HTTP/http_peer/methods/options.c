#include "options.h"

#include <stdio.h>

HTTP_Status HTTPOptions(Request *req, HTTP_Response *res) {
    (void) req;

    if (res == NULL || res->headerList == NULL) {
        return STATUS_500;
    }

    addResponseHeader(res->headerList, "Allow", "GET, HEAD, POST, PUT, DELETE, TRACE, CONNECT, OPTIONS");
    addResponseHeader(res->headerList, "Content-Length", "0");

    res->content = NULL;
    res->contentLength = 0;

    return STATUS_200;
}
