#include "response.h"
#include "../utils/enumToString.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printResponse(HTTP_Response *res) {
    char headerBuffer[4096];
    size_t offset = 0;
    
    // Status line
    offset += snprintf(headerBuffer + offset, sizeof(headerBuffer) - offset,
        "%s %s %s\r\n",
        versionToString(res->httpVersion),
        statusToString(res->status),
        statusToReasonPhrase(res->status)
    );
    
    // Headers
    for (size_t i = 0; i < res->headerList->count; i++) {
        offset += snprintf(headerBuffer + offset, sizeof(headerBuffer) - offset,
            "%s: %s\r\n",
            res->headerList->headers[i].name,
            res->headerList->headers[i].value
        );
    }
    
    offset += snprintf(headerBuffer + offset, sizeof(headerBuffer) - offset, "\r\n");
    
    printf("\n\n%s", headerBuffer);
    if (res->content != NULL) {
        // Con fwrite para que funcione con binarios
        fwrite(res->content, 1, res->contentLength, stdout);
    }
}

HeaderList *createHeaderList(){
    HeaderList *headerList = malloc(sizeof(HeaderList));
    headerList->count = 0;
    
    return headerList;
}

HTTP_Response *createHTTPResponse(){
    HTTP_Response *res = malloc(sizeof(HTTP_Response));

    res->status = STATUS_NULL;
    res->httpVersion = VERSION_HTTP1;
    
    res->headerList = createHeaderList();
    res->content = NULL;
    res->contentLength = 0;
    
    return res;
}

int addHeader(HeaderList *list , const char *name , const char *value){
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

void freeHeaderList(HeaderList *list){
    if (list == NULL) return;

    for (size_t i = 0; i < list->count; i++){
        free(list->headers[i].name);
        free(list->headers[i].value);
    }

    free(list);
}

void freeResponse(HTTP_Response *res){
    if (res == NULL) return;

    free(res->content);
    freeHeaderList(res->headerList);
    free(res);
}
