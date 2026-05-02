#ifndef TRACE_H
#define TRACE_H

#include "../../structs/request.h"
#include "../../structs/response.h"

HTTP_Status HTTPTrace(Request *req, HTTP_Response *res);

#endif
