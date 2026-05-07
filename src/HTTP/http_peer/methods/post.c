#include "post.h"
#include "../utils/readFile.h"

#include <stdio.h>
#include <string.h>

HTTP_Status HTTPPost(Request *req , HTTP_Response *res){
    char fullpath[512];

    if (req->requestURI == NULL || strlen(req->requestURI) <= 1) return STATUS_400;
    if (strstr(req->requestURI, "..") != NULL) return STATUS_400;
    if (buildDocumentPath(req->requestURI, fullpath, sizeof(fullpath)) != 0) return STATUS_400;

    FILE *f = fopen(fullpath , "wb");
    if (f == NULL) return STATUS_500;

    fwrite(req->body , 1 , req->bodyLength, f);
    fclose(f);

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
