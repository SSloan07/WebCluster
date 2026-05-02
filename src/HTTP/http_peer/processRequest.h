#ifndef PROCESS_REQUEST_H
#define PROCESS_REQUEST_H

#include "structs/response.h"
#include "structs/request.h"

HTTP_Status http_peer_process_request(Request *req, HTTP_Response *res);

#endif
