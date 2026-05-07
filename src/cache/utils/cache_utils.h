#ifndef CACHE_UTILS_H
#define CACHE_UTILS_H

#include "../cacheTypes.h"
#include <stddef.h>

// Generates a simple and stable hash to turn the cache key into a file name.
unsigned long cache_hash_key(const char *text);

// Creates the cache directory if it does not exist and validates that it is actually a directory.
int ensure_cache_directory(const char *cache_dir);

// Creates the index file if it does not exist yet.
int ensure_index_file(const char *index_path);

// Stores the full HTTP response on disk.
int write_cache_file(const char *file_path, const char *response_data, size_t response_size);

// Converts one index line into a cache_entry_t struct.
int parse_index_line(const char *line, cache_entry_t *entry);

// Rewrites the index using a temporary file to remove a key or expired entries.
int rewrite_index(cache_store_t *store, const char *cache_key, int remove_expired_only, int *removed_any);

// Background thread that periodically performs active cleanup.
void *cache_cleaner_thread(void *arg);

#endif
