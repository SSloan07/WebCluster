#ifndef REQUEST_H
#define REQUEST_H

#include <stdlib.h>
#define MAX_HEADERS 128

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
    HEADER_ACCEPT_CHARSET,
    HEADER_ACCEPT_ENCODING,
    HEADER_ACCEPT_LANGUAGE,
    HEADER_AUTHORIZATION,
    HEADER_COOKIE,
    HEADER_CACHE_CONTROL,
    HEADER_EXPECT,
    HEADER_FROM,
    HEADER_IF_MATCH,
    HEADER_IF_MODIFIED_SINCE,
    HEADER_IF_NONE_MATCH,
    HEADER_IF_RANGE,
    HEADER_IF_UNMODIFIED_SINCE,
    HEADER_MAX_FORWARDS,
    HEADER_PROXY_AUTHORIZATION,
    HEADER_RANGE,
    HEADER_REFERER,
    HEADER_TE,
    HEADER_UNKNOWN,

} Request_Header_Name;

typedef enum {
    VERSION_HTTP1,
    VERSION_UNKNOWN,
    VERSION_NULL,
} HTTP_Version;

typedef struct {
    Request_Header_Name type;
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
    char *body;
    size_t bodyLength;
} Request;

void printRequest(Request *req);
Request *createRequest();
void freeRequest (Request *req);
Request_HeaderList *createRequestHeaderList();
int addRequestHeader(Request_HeaderList *list , Request_Header_Name type , const char *name , const char *value);
void freeRequestHeaderList(Request_HeaderList *list);
Request_Header *searchHeader(Request_HeaderList *headerList , Request_Header_Name requestHeader);

#endif
