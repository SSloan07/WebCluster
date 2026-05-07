#ifndef POST_H
#define POST_H

#include "../../structs/request.h"
#include "../../structs/response.h"

HTTP_Status HTTPPost(Request *req , HTTP_Response *res);

#endif