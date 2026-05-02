#ifndef MANAGE_CACHE_H
#define MANAGE_CACHE_H

#include "../HTTP/HttpParser.h"
#include "cacheTypes.h"

cache_result_t cache_init(cache_store_t *store, const char *cache_dir, int default_ttl);

cache_result_t cache_lookup(cache_store_t *store, const char *cache_key, char *out_file_path, size_t out_size);

cache_result_t cache_save(cache_store_t *store, const char *cache_key, const char *response_data, size_t response_size);

cache_result_t cache_delete(cache_store_t *store, const char *cache_key);

cache_result_t cache_cleanup_expired(cache_store_t *store);

cache_result_t cache_start_cleaner_thread(cache_store_t *store);

int http_request_is_cacheable(const Request *request);

int http_build_cache_key(const Request *request, char *out, size_t out_size);

void *cache_cleaner_thread(void *arg);

#endif
