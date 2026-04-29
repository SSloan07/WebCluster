#include "processRequest.h"
#include "methods/get.h"
#include "methods/head.h"
#include "methods/post.h"

#include <string.h>
#include <stdio.h>


HTTP_Status processRequest(Request *req , HTTP_Response *res){
    
    if(req->method == METHOD_UNKNOWN) return STATUS_400;
    if(req->httpVersion == VERSION_UNKNOWN) return STATUS_505;

    res->httpVersion = req->httpVersion;

    switch (req->method){
        case METHOD_GET:
            return HTTPGet(req , res);

        case METHOD_HEAD:
            return HTTPHead(req , res);

        case METHOD_POST:
            return HTTPPost(req , res);
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