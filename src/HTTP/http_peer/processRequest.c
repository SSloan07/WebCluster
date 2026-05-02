#include "processRequest.h"
#include "methods/delete.h"
#include "methods/get.h"
#include "methods/head.h"
#include "methods/post.h"
#include "methods/trace.h"
#include "methods/put.h"

#include <stdio.h>
#include <string.h>

HTTP_Status http_peer_process_request(Request *req, HTTP_Response *res) {
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

        case METHOD_PUT:
            return HTTPPut(req, res);

        case METHOD_DELETE:
            return HTTPDelete(req, res);

        case METHOD_TRACE:
            return HTTPTrace(req, res);

        case METHOD_CONNECT:
            break;

        case METHOD_OPTIONS:
            break;

        default:
            return STATUS_400;
    }

    return 0;
}
