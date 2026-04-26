#include "stringToEnum.h"

#include <string.h>
#include <strings.h>

HTTP_Method getHTTPMethod(const char *strMethod){

    if(strcmp(strMethod , "GET") == 0) return METHOD_GET;
    if(strcmp(strMethod , "HEAD") == 0) return METHOD_HEAD;
    if(strcmp(strMethod , "POST") == 0) return METHOD_POST;
    if(strcmp(strMethod , "PUT") == 0) return METHOD_PUT;
    if(strcmp(strMethod , "DELETE") == 0) return METHOD_DELETE;
    if(strcmp(strMethod , "TRACE") == 0) return METHOD_TRACE;
    if(strcmp(strMethod , "CONNECT") == 0) return METHOD_CONNECT;
    if(strcmp(strMethod , "OPTIONS") == 0) return METHOD_OPTIONS;

    return METHOD_UNKNOWN;
}

HTTP_Version getHTTPVersion(const char *strVersion){

    if(strcmp(strVersion , "HTTP/1.1") == 0) return VERSION_HTTP1;

    return VERSION_UNKNOWN;
}

Request_Header_Name getRequestHeader(const char *strHeader){
    
    if(strcasecmp(strHeader , "Host") == 0) return HEADER_HOST; // Ignorar las mayusculas

    return HEADER_UNKNOWN;
}

