#include "get.h"
#include "../utils/readFile.h"
#include "../utils/getDate.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

HTTP_Status HTTPGet(RequestLine *req , HTTP_Response *res){

    // Sacar la ruta por defecto (/index.html)
    if(strcmp(req->requestURI , "/") == 0){
        char *defaultURI = "/index.html";

        req->requestURI = realloc(req->requestURI , strlen(defaultURI) + 1);
        strcpy(req->requestURI, defaultURI);
    }

    size_t contentSize;
    char *content = readFile(req->requestURI , &contentSize);
    
    // El archivo no existe
    if (content == NULL) return STATUS_404;

    // El archivo existe
    res->content = content; 
    res->contentLength = contentSize;

    // Añadir headers
    const char *date = "Date";
    const char *contentLength = "Content-Length";
    const char *contentType = "Content-Type";
    const char *server = "Server";

    char buffer[64];
    size_t bufferSize = sizeof(buffer);
    getDate(buffer , bufferSize);

    addHeader(res->headerList , date , buffer);

    char contentLengthStr[32];
    snprintf(contentLengthStr , sizeof(contentLengthStr) , "%zu", res->contentLength);
    addHeader(res->headerList , contentLength , contentLengthStr);

    addHeader(res->headerList , contentType , getContentType(req->requestURI));

    addHeader(res->headerList , server , "El server de los mas papus");
    
    return STATUS_200;
    
}