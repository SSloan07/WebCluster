#ifndef MANAGE_CACHE_H
#define MANAGE_CACHE_H
#include "../HTTP/HttpParser.h"
#include "cacheTypes.h"


int http_request_is_cacheable(const Request *request);
    
int http_build_cache_key(const Request *request, char *out, size_t out_size);

#endif