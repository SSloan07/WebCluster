#ifndef GET_H
#define GET_H

#include "../structs/requestLine.h"
#include "../structs/response.h"

HTTP_Status HTTPGet(RequestLine *req , HTTP_Response *res);

#endif