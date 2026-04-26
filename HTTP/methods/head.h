#ifndef HEAD_H
#define HEAD_H

#include "../structs/request.h"
#include "../structs/response.h"

HTTP_Status HTTPHead(RequestLine *req, HTTP_Response *res);

#endif