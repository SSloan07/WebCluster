#include "head.h"
#include "../utils/readFile.h"

#include <string.h>
#include <stdlib.h>

HTTP_Status HTTPHead(RequestLine *req) {
    // Sacar la ruta por defecto (/index.html)
    if(strcmp(req->requestURI , "/") == 0){
        char *defaultURI = "/index.html";

        req->requestURI = realloc(req->requestURI , strlen(defaultURI) + 1);
        strcpy(req->requestURI, defaultURI);
    }

    if(fileExist(req->requestURI) == -1) return STATUS_404;

    // Implementar la logica de los headers...
    
    return STATUS_200;
}