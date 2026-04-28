#ifndef REQUEST_LINE_H
#define REQUEST_LINE_H

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
    VERSION_HTTP1,
    VERSION_UNKNOWN,
    VERSION_NULL,
} HTTP_Version;

typedef struct {
    HTTP_Method method;
    char *requestURI;
    HTTP_Version httpVersion;
} RequestLine;

void printRequestLine(RequestLine *req);
RequestLine *createRequestLine();
void freeRequestLine (RequestLine *req);

#endif