#include "delete.h"
#include "../utils/readFile.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

HTTP_Status HTTPDelete(Request *req, HTTP_Response *res) {
    char fullpath[512];

    if (req->requestURI == NULL || strlen(req->requestURI) <= 1) return STATUS_400;
    if (strstr(req->requestURI, "..") != NULL) return STATUS_400;
    if (buildDocumentPath(req->requestURI, fullpath, sizeof(fullpath)) != 0) return STATUS_400;

    if (access(fullpath, F_OK) != 0) {
        return STATUS_404;
    }

    if (remove(fullpath) != 0) {
        return STATUS_500;
    }

    res->content = strdup("File deleted successfully\r\n");
    if (res->content == NULL) return STATUS_500;

    res->contentLength = strlen(res->content);

    char lenContent[32];
    snprintf(lenContent, sizeof(lenContent), "%zu", res->contentLength);

    addResponseHeader(res->headerList, "Content-Type", "text/plain");
    addResponseHeader(res->headerList, "Content-Length", lenContent);

    return STATUS_200;
}
