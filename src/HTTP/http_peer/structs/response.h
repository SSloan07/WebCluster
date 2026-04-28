#ifndef RESPONSE_H
#define RESPONSE_H

#include "requestLine.h"
#include <stdlib.h>

#define MAX_HEADERS 64

typedef enum {
    STATUS_200,
    STATUS_400,
    STATUS_404,
    STATUS_505,
    STATUS_NULL,
} HTTP_Status;

typedef struct {
    char *name;
    char *value;
} Header;

typedef struct {
    Header headers[MAX_HEADERS];
    size_t count;
} HeaderList;

typedef struct {
    HTTP_Status status;
    HTTP_Version httpVersion;
    HeaderList *headerList;
    char *content;
    size_t contentLength; 
} HTTP_Response;

void printResponse( HTTP_Response *res);
void freeResponse(HTTP_Response *res);
HTTP_Response *createHTTPResponse();
HeaderList *createHeaderList();
int addHeader(HeaderList *list , const char *name , const char *value);
void freeHeaderList(HeaderList *list);

#endif