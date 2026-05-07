#ifndef CACHE_UTILS_H
#define CACHE_UTILS_H

#include "../cacheTypes.h"
#include <stddef.h>

// Genera un hash simple y estable para convertir la cache_key en nombre de archivo.
unsigned long cache_hash_key(const char *text);

// Crea el directorio de cache si no existe y valida que realmente sea un directorio. 
int ensure_cache_directory(const char *cache_dir);

// Crea el archivo indice si todavia no existe. 
int ensure_index_file(const char *index_path);

// Guarda la respuesta HTTP completa en disco
int write_cache_file(const char *file_path, const char *response_data, size_t response_size);

// Convierte una linea del indice al struct cache_entry_t.
int parse_index_line(const char *line, cache_entry_t *entry);

// Reescribe el indice usando un archivo temporal para eliminar una clave o entradas expiradas.
int rewrite_index(cache_store_t *store, const char *cache_key, int remove_expired_only, int *removed_any);

// Hilo en segundo plano que ejecuta limpieza activa periodicamente.
void *cache_cleaner_thread(void *arg);

#endif
