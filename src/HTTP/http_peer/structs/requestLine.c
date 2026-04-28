#include "requestLine.h"
#include "../utils/enumToString.h"

#include <stdio.h>
#include <stdlib.h>

void printRequestLine(RequestLine *req){
    printf("\n\n\n\n\n\n--------------Estos son los datos que se parsearon a la request---------------------\n");
    printf("Metodo: %s\n" , methodToString(req->method));
    printf("requestURI: %s\n" , req->requestURI);
    printf("Version de HTTP: %s\n" , versionToString(req->httpVersion));
}

RequestLine *createRequestLine(){
    RequestLine *req = malloc(sizeof(RequestLine));

    req->method = METHOD_NULL;
    req->requestURI = NULL;
    req->httpVersion = VERSION_NULL;

    return req;
}

void freeRequestLine (RequestLine *req){
    if(req == NULL) return;

    free(req->requestURI);
    free(req);
}
