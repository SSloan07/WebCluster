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
    HEADER_CONTENT_TYPE,
    HEADER_CONTENT_LENGTH,
    HEADER_USER_AGENT,
    HEADER_ACCEPT,
    HEADER_UNKNOWN,
} Request_Header_Name;

typedef enum {
    VERSION_HTTP1,
    VERSION_UNKNOWN,
    VERSION_NULL,
} HTTP_Version;

typedef struct {
    Request_Header_Name name;
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
    char *body;
    size_t bodyLength;
} Request;

void printRequest(Request *req);
Request *createRequest();
void freeRequest (Request *req);
Request_HeaderList *createRequestHeaderList();
int addRequestHeader(Request_HeaderList *list , Request_Header_Name name , const char *value);
void freeRequestHeaderList(Request_HeaderList *list);
Request_Header *searchHeader(Request_HeaderList *headerList , Request_Header_Name requestHeader);

#endif
