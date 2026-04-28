#include "request.h"
#include "../utils/enumToString.h"

#include <stdio.h>
#include <string.h>

void printRequest(Request *req) {

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
            headerToString(req->headerList->headers[i].name),
            req->headerList->headers[i].value
        );
    }
    
    offset += snprintf(headerBuffer + offset, sizeof(headerBuffer) - offset, "\r\n");
    
    printf("\n\n%s", headerBuffer);
    
    if (req->body != NULL) {
        fwrite(req->body, 1, req->bodyLength, stdout);
    }
}

Request *createRequest(){
    Request *req = malloc(sizeof(Request));

    req->method = METHOD_NULL;
    req->requestURI = NULL;
    req->headerList = createRequestHeaderList();
    req->httpVersion = VERSION_NULL;
    req->body = NULL;
    req->bodyLength = 0;

    return req;
}

Request_HeaderList *createRequestHeaderList(){
    Request_HeaderList *headerList = malloc(sizeof(Request_HeaderList));
    headerList->count = 0;
    
    return headerList;
}

int addRequestHeader(Request_HeaderList *list , Request_Header_Name name , const char *value){
    if(list == NULL || value == NULL) return -1;
    if(list->count >= MAX_HEADERS) return -1;

    // Crear una copia de estos, por si *name o *value cambian o desaparecen en un siguiente request
    // Aqui se hace un malloc y un strcpy
    char *valueCopy = strdup(value);

    list->headers[list->count].name = name;
    list->headers[list->count].value = valueCopy;
    list->count++;

    return 0;
}

void freeRequestHeaderList(Request_HeaderList *list){
    if (list == NULL) return;

    for (size_t i = 0; i < list->count; i++){
        free(list->headers[i].value);
    }

    free(list);
}

void freeRequest (Request *req){
    if(req == NULL) return;

    free(req->requestURI);
    freeRequestHeaderList(req->headerList);
    free(req->body);
    free(req);
}

Request_Header *searchHeader(Request_HeaderList *list , Request_Header_Name requestHeader) {
    if (list == NULL || list->count == 0) return NULL;
    
    for (size_t i = 0 ; i < list->count ; i++)
    {
        if(list->headers[i].name == requestHeader){
            return &list->headers[i];
        }
    }
    return NULL;
}
