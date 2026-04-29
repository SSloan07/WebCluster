#ifndef RESPONSE_H
#define RESPONSE_H

#include "request.h"
#include <stdlib.h>

#define MAX_HEADERS 64

typedef enum {
    STATUS_200,
    STATUS_201,
    STATUS_400,
    STATUS_404,
    STATUS_405,
    STATUS_500,
    STATUS_505,
    STATUS_NULL,
} HTTP_Status;

typedef struct {
    char *name;
    char *value;
} Response_Header;

typedef struct {
    Response_Header headers[MAX_HEADERS];
    size_t count;
} Response_HeaderList;

typedef struct {
    HTTP_Status status;
    HTTP_Version httpVersion;
    Response_HeaderList *headerList;
    char *content;
    size_t contentLength; 
} HTTP_Response;

void printResponse( HTTP_Response *res);
void freeResponse(HTTP_Response *res);
HTTP_Response *createHTTPResponse();
Response_HeaderList *createResponseHeaderList();
int addResponseHeader(Response_HeaderList *list , const char *name , const char *value);
void freeResponseHeaderList(Response_HeaderList *list);

#endif