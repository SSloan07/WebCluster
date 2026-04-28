#ifndef HEAD_H
#define HEAD_H

#include "../structs/requestLine.h"
#include "../structs/response.h"

HTTP_Status HTTPHead(RequestLine *req);

#endif