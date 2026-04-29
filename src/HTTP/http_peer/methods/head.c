#include "head.h"
#include "../utils/readFile.h"
#include "../utils/getDate.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

HTTP_Status HTTPHead(Request *req , HTTP_Response *res) {
    // Sacar la ruta por defecto (/index.html)
    if(strcmp(req->requestURI , "/") == 0){
        char *defaultURI = "/index.html";

        req->requestURI = realloc(req->requestURI , strlen(defaultURI) + 1);
        strcpy(req->requestURI, defaultURI);
    }

    // Leer el contenido y sacar el tamaño
    size_t contentSize;
    char *content = readFile(req->requestURI , &contentSize);
    if(content == NULL) return STATUS_404;

    // Añadir headers
    const char *date = "Date";
    const char *contentLength = "Content-Length";
    const char *contentType = "Content-Type";
    const char *server = "Server";

    // Fecha
    char buffer[64];
    size_t bufferSize = sizeof(buffer);
    getDate(buffer , bufferSize);
    addResponseHeader(res->headerList , date , buffer);

    // Bytes del archivo
    char contentLengthStr[32];
    snprintf(contentLengthStr , sizeof(contentLengthStr) , "%zu", res->contentLength);
    addResponseHeader(res->headerList , contentLength , contentLengthStr);

    // Tipo de archivo
    addResponseHeader(res->headerList , contentType , getContentType(req->requestURI));

    // Servidor
    addResponseHeader(res->headerList , server , "El server de los mas papus");
    
    return STATUS_200;
}