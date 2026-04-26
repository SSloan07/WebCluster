#include "processRequest.h"
#include "methods/get.h"
#include "methods/head.h"

#include <string.h>
#include <stdio.h>


HTTP_Status processRequest(RequestLine *req , HTTP_Response *res){
    
    if(req->method == METHOD_UNKNOWN) return STATUS_400;
    if(req->httpVersion == VERSION_UNKNOWN) return STATUS_505;

    switch (req->method){
        case METHOD_GET:
            return HTTPGet(req , res);

        case METHOD_HEAD:
            return HTTPHead(req , res);

        case METHOD_POST:
            break;

        case METHOD_PUT:
            break;

        case METHOD_DELETE:
            break;

        case METHOD_TRACE:
            break;

        case METHOD_CONNECT:
            break;

        case METHOD_OPTIONS:

            break;
        
        default:
            return STATUS_400;
    }

    return 0;
}