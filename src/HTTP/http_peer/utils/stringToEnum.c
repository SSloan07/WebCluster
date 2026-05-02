#include "stringToEnum.h"

#include <string.h>

HTTP_Method getHTTPMethod(const char *strMethod){

    if(strcmp(strMethod , "GET") == 0) return METHOD_GET;
    if(strcmp(strMethod , "HEAD") == 0) return METHOD_HEAD;
    if(strcmp(strMethod , "POST") == 0) return METHOD_POST;
    if(strcmp(strMethod , "PUT") == 0) return METHOD_PUT;
    if(strcmp(strMethod , "DELETE") == 0) return METHOD_DELETE;
    if(strcmp(strMethod , "TRACE") == 0) return METHOD_TRACE;
    if(strcmp(strMethod , "CONNECT") == 0) return METHOD_CONNECT;
    // if(strcmp(strMethod , "OPTIONS") == 0) return METHOD_OPTIONS;

    return METHOD_UNKNOWN;
}

HTTP_Version getHTTPVersion(const char *strVersion){

    if(strcmp(strVersion , "HTTP/1.1") == 0) return VERSION_HTTP1;

    return VERSION_UNKNOWN;
}

Request_Header_Name getRequestHeader(const char *strHeader){
    
    if (strcasecmp(strHeader, "Accept") == 0) return HEADER_ACCEPT;
    if (strcasecmp(strHeader, "Accept-Charset") == 0) return HEADER_ACCEPT_CHARSET;
    if (strcasecmp(strHeader, "Accept-Encoding") == 0) return HEADER_ACCEPT_ENCODING;
    if (strcasecmp(strHeader, "Accept-Language") == 0) return HEADER_ACCEPT_LANGUAGE;
    if (strcasecmp(strHeader, "Authorization") == 0) return HEADER_AUTHORIZATION;
    if (strcasecmp(strHeader, "Expect") == 0) return HEADER_EXPECT;
    if (strcasecmp(strHeader, "From") == 0) return HEADER_FROM;
    if (strcasecmp(strHeader, "Host") == 0) return HEADER_HOST;
    if (strcasecmp(strHeader, "If-Match") == 0) return HEADER_IF_MATCH;
    if (strcasecmp(strHeader, "If-Modified-Since") == 0) return HEADER_IF_MODIFIED_SINCE;
    if (strcasecmp(strHeader, "If-None-Match") == 0) return HEADER_IF_NONE_MATCH;
    if (strcasecmp(strHeader, "If-Range") == 0) return HEADER_IF_RANGE;
    if (strcasecmp(strHeader, "If-Unmodified-Since") == 0) return HEADER_IF_UNMODIFIED_SINCE;
    if (strcasecmp(strHeader, "Max-Forwards") == 0) return HEADER_MAX_FORWARDS;
    if (strcasecmp(strHeader, "Proxy-Authorization") == 0) return HEADER_PROXY_AUTHORIZATION;
    if (strcasecmp(strHeader, "Range") == 0) return HEADER_RANGE;
    if (strcasecmp(strHeader, "Referer") == 0) return HEADER_REFERER;
    if (strcasecmp(strHeader, "TE") == 0) return HEADER_TE;
    if (strcasecmp(strHeader, "User-Agent") == 0) return HEADER_USER_AGENT;
    if (strcasecmp(strHeader, "Cookie") == 0) return HEADER_COOKIE;
    if (strcasecmp(strHeader, "Cache-Control") == 0) return HEADER_CACHE_CONTROL;
    
    if (strcasecmp(strHeader, "Content-Type") == 0) return HEADER_CONTENT_TYPE;
    if (strcasecmp(strHeader, "Content-Length") == 0) return HEADER_CONTENT_LENGTH;
    
    return HEADER_UNKNOWN;
}
