#include "request.h"
#include "../http_peer/utils/enumToString.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
        const char *headerName = req->headerList->headers[i].name;

        if (headerName == NULL) {
            headerName = headerToString(req->headerList->headers[i].type);
        }

        offset += snprintf(headerBuffer + offset, sizeof(headerBuffer) - offset,
            "%s: %s\r\n",
            headerName,
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
    if (req == NULL) return NULL;

    req->method = METHOD_NULL;
    req->requestURI = NULL;
    req->headerList = createRequestHeaderList();
    if (req->headerList == NULL) {
        free(req);
        return NULL;
    }
    req->httpVersion = VERSION_NULL;
    req->body = NULL;
    req->bodyLength = 0;

    return req;
}

Request_HeaderList *createRequestHeaderList(){
    Request_HeaderList *headerList = malloc(sizeof(Request_HeaderList));
    if (headerList == NULL) return NULL;

    headerList->count = 0;
    
    return headerList;
}

int addRequestHeader(Request_HeaderList *list , Request_Header_Name type , const char *name , const char *value){
    if(list == NULL || name == NULL || value == NULL) return -1;
    if(list->count >= MAX_HEADERS) return -1;

    char *nameCopy = strdup(name);
    char *valueCopy = strdup(value);

    if (nameCopy == NULL || valueCopy == NULL) {
        free(nameCopy);
        free(valueCopy);
        return -1;
    }

    list->headers[list->count].type = type;
    list->headers[list->count].name = nameCopy;
    list->headers[list->count].value = valueCopy;
    list->count++;

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
        if(list->headers[i].type == requestHeader){
            return &list->headers[i];
        }
    }
    return NULL;
}
