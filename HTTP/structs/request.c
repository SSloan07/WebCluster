#include "request.h"
#include "../utils/enumToString.h"

#include <stdio.h>
#include <string.h>

void printRequest(RequestLine *req) {

    char headerBuffer[4096];
    size_t offset = 0;
    
    // Request line
    offset += snprintf(headerBuffer + offset, sizeof(headerBuffer) - offset,
        "%s %s %s\r\n",
        methodToString(req->method),
        req->requestURI,
        versionToString(req->httpVersion)
    );
    
    // Headers
    for (size_t i = 0; i < req->headerList->count; i++) {
        offset += snprintf(headerBuffer + offset, sizeof(headerBuffer) - offset,
            "%s: %s\r\n",
            req->headerList->headers[i].name,
            req->headerList->headers[i].value
        );
    }
    
    offset += snprintf(headerBuffer + offset, sizeof(headerBuffer) - offset, "\r\n");
    
    printf("\n\n%s", headerBuffer);
    // Puede ser util para implementar el body en un request POST
    // if (req->content != NULL) {
    //     // Con fwrite para que funcione con binarios
    //     fwrite(req->content, 1, req->contentLength, stdout);
    // }
}

RequestLine *createRequest(){
    RequestLine *req = malloc(sizeof(RequestLine));

    req->method = METHOD_NULL;
    req->requestURI = NULL;
    req->headerList = createRequestHeaderList();
    req->httpVersion = VERSION_NULL;

    return req;
}

Request_HeaderList *createRequestHeaderList(){
    Request_HeaderList *headerList = malloc(sizeof(Request_HeaderList));
    headerList->count = 0;
    
    return headerList;
}

int addRequestHeader(Request_HeaderList *list , const char *name , const char *value){
    if(list == NULL) return -1;
    if(list->count >= MAX_HEADERS) return -1;

    // Crear una copia de estos, por si *name o *value cambian o desaparecen en un siguiente request
    // Aqui se hace un malloc y un strcpy
    char *nameCopy = strdup(name);
    char *valueCopy = strdup(value);

    list->headers[list->count].name = nameCopy;
    list->headers[list->count].value = valueCopy;
    list->count = list->count + 1;

    return 0;
}

void freeRequestHeaderList(Request_HeaderList *list){
    if (list == NULL) return;

    for (size_t i = 0; i < list->count; i++){
        free(list->headers[i].name);
        free(list->headers[i].value);
    }

    free(list);
}

void freeRequest (RequestLine *req){
    if(req == NULL) return;

    free(req->requestURI);
    freeRequestHeaderList(req->headerList);
    free(req);
}
