#ifndef CACHE_TYPES_H
#define CACHE_TYPES_H

#include <stddef.h>
#include <time.h>
#include 

#define CACHE_KEY_MAX 2048
#define CACHE_PATH_MAX 1024
#define CACHE_MAX_ENTRIES 100

typedef struct {
    char key[CACHE_KEY_MAX];
    char file_path[CACHE_PATH_MAX];
    time_t expires_at;
    int in_use;
} cache_entry_t;



int http_request_is_cacheable(const Request *request);
    

int http_build_cache_key(const Request *request, char *out, size_t out_size);

#endif
