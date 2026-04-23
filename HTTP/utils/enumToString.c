#include "enumToString.h"

const char *methodToString(HTTP_Method method){
    if(method == METHOD_GET) return "GET"; 
    if(method == METHOD_POST) return "POST"; 
    if(method == METHOD_HEAD) return "HEAD"; 
     return "UNKNOWN o no lo he mapeado"; 
}

const char *versionToString(HTTP_Version version){
    if(version == VERSION_HTTP1) return "HTTP/1.1"; 
     return "UNKNOWN"; 
}

const char *statusToString(HTTP_Status status){
    if(status == STATUS_200) return "200";
    if(status == STATUS_404) return "404";
    if(status == STATUS_400) return "400";
    if(status == STATUS_505) return "505";
    return "999";
}

const char *statusToReasonPhrase(HTTP_Status status) {
    if(status == STATUS_200) return "OK";
    if(status == STATUS_404) return "Not Found";
    if(status == STATUS_400) return "Bad Request";
    if(status == STATUS_505) return "HTTP Version not supported";
    return "nadita";
}

