#ifndef PROCESS_REQUEST_H
#define PROCESS_REQUEST_H

#include "structs/response.h"
#include "structs/requestLine.h"

HTTP_Status processRequest(RequestLine *req , HTTP_Response *res );

#endif