#include "put.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define UPLOAD_DIR "./files"

HTTP_Status HTTPPut(Request *req, HTTP_Response *res) {
    if (strncmp(req->requestURI, "/files/", 7) != 0) {
        return STATUS_405;
    }

    const char *filename = req->requestURI + 7;

    if (strlen(filename) == 0) return STATUS_400;
    if (strstr(filename, "..") != NULL || strchr(filename, '/')) return STATUS_400;

    char fullpath[512];
    int bytesWritten = snprintf(fullpath, sizeof(fullpath), "%s/%s", UPLOAD_DIR, filename);
    if (bytesWritten < 0 || bytesWritten >= (int) sizeof(fullpath)) return STATUS_400;

    mkdir(UPLOAD_DIR, 0755);

    int existed_before = (access(fullpath, F_OK) == 0);

    FILE *f = fopen(fullpath, "wb");
    if (f == NULL) return STATUS_500;

    fwrite(req->body, 1, req->bodyLength, f);
    fclose(f);

    if (existed_before) {
        res->content = strdup("File updated successfully\r\n");
        if (res->content == NULL) return STATUS_500;

        res->contentLength = strlen(res->content);

        char lenContent[32];
        snprintf(lenContent, sizeof(lenContent), "%zu", res->contentLength);

        addResponseHeader(res->headerList, "Content-Type", "text/plain");
        addResponseHeader(res->headerList, "Content-Length", lenContent);

        return STATUS_200;
    }

    res->content = strdup("File created successfully\r\n");
    if (res->content == NULL) return STATUS_500;

    res->contentLength = strlen(res->content);

    char lenContent[32];
    snprintf(lenContent, sizeof(lenContent), "%zu", res->contentLength);

    addResponseHeader(res->headerList, "Content-Type", "text/plain");
    addResponseHeader(res->headerList, "Content-Length", lenContent);
    addResponseHeader(res->headerList, "Location", req->requestURI);

    return STATUS_201;
}
