#include "get.h"
#include "../utils/readFile.h"
#include "../utils/getDate.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

HTTP_Status HTTPGet(Request *req , HTTP_Response *res){

    // Resolve the default path (/index.html)
    if(strcmp(req->requestURI , "/") == 0){
        char *defaultURI = "/index.html";

        req->requestURI = realloc(req->requestURI , strlen(defaultURI) + 1);
        strcpy(req->requestURI, defaultURI);
    }

    size_t contentSize;
    char *content = readFile(req->requestURI , &contentSize);
    
    // The file does not exist
    if (content == NULL) return STATUS_404;

    // The file exists
    res->content = content; 
    res->contentLength = contentSize;

    // Add headers
    const char *date = "Date";
    const char *contentLength = "Content-Length";
    const char *contentType = "Content-Type";
    const char *server = "Server";

    // Date
    char buffer[64];
    size_t bufferSize = sizeof(buffer);
    getDate(buffer , bufferSize);
    addResponseHeader(res->headerList , date , buffer);

    // File size in bytes
    char contentLengthStr[32];
    snprintf(contentLengthStr , sizeof(contentLengthStr) , "%zu", res->contentLength);
    addResponseHeader(res->headerList , contentLength , contentLengthStr);

    // File type
    addResponseHeader(res->headerList , contentType , getContentType(req->requestURI));

    // Server
    addResponseHeader(res->headerList , server , "El server de los mas papus");
    
    return STATUS_200;
    
}
