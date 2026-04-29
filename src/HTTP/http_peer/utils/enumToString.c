#include "enumToString.h"

const char *methodToString(HTTP_Method method){
    if(method == METHOD_GET) return "GET"; 
    if(method == METHOD_POST) return "POST"; 
    if(method == METHOD_HEAD) return "HEAD"; 
     return "UNKNOWN METHOD"; 
}

const char *versionToString(HTTP_Version version){
    if(version == VERSION_HTTP1) return "HTTP/1.1"; 
     return "UNKNOWN VERSION"; 
}

const char *statusToString(HTTP_Status status){
    if(status == STATUS_200) return "200";
    if(status == STATUS_201) return "201";
    if(status == STATUS_404) return "404";
    if(status == STATUS_405) return "405";
    if(status == STATUS_400) return "400";
    if(status == STATUS_500) return "500";
    if(status == STATUS_505) return "505";
    return "777"; // Para la buena suerte
}

const char *statusToReasonPhrase(HTTP_Status status){
    if(status == STATUS_200) return "OK";
    if(status == STATUS_201) return "Created";
    if(status == STATUS_404) return "Not Found";
    if(status == STATUS_405) return "Method Not Allowed";
    if(status == STATUS_400) return "Bad Request";
    if(status == STATUS_500) return "Internal Server Error";
    if(status == STATUS_505) return "HTTP Version not supported";
    return "Status code not supported";
}

const char *headerToString(Request_Header_Name name){
    if(name == HEADER_HOST) return "Host";
    if(name == HEADER_CONTENT_TYPE) return "Content-Type";
    if(name == HEADER_CONTENT_LENGTH) return "Content-Length";
    if(name == HEADER_USER_AGENT) return "User-Agent";
    if(name == HEADER_ACCEPT) return "Accept";
    return "UNKOWN HEADER";
}

