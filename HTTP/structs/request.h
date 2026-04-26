#ifndef REQUEST_H
#define REQUEST_H

#include <stdlib.h>
#define MAX_HEADERS 64

typedef enum {
    METHOD_GET,
    METHOD_HEAD,
    METHOD_POST,
    METHOD_PUT,
    METHOD_DELETE,
    METHOD_TRACE,
    METHOD_CONNECT,
    METHOD_OPTIONS,
    METHOD_UNKNOWN,
    METHOD_NULL,
} HTTP_Method;

typedef enum {
    HEADER_HOST,
    HEADER_UNKNOWN,
} Request_Header_Name;

typedef enum {
    VERSION_HTTP1,
    VERSION_UNKNOWN,
    VERSION_NULL,
} HTTP_Version;

typedef struct {
    char *name;
    char *value;
} Request_Header;

typedef struct {
    Request_Header headers[MAX_HEADERS];
    size_t count;
} Request_HeaderList;

typedef struct {
    Request_HeaderList *headerList;
    HTTP_Method method;
    char *requestURI;
    HTTP_Version httpVersion;
} RequestLine;

void printRequest(RequestLine *req);
RequestLine *createRequest();
void freeRequest (RequestLine *req);
Request_HeaderList *createRequestHeaderList();
int addRequestHeader(Request_HeaderList *list , const char *name , const char *value);
void freeRequestHeaderList(Request_HeaderList *list);

#endif