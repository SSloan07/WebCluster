#include "Manage_cache.h"
#include "../HTTP/http_peer/utils/enumToString.h"


int http_build_cache_key(const Request *request, char *out, size_t out_size) {
    if (!request || !out || out_size == 0 || !request->requestURI) {
        return -1;
    }

    const char *host = http_request_get_header(request, "Host");
    if (!host) host = "nohost";

    const char *method = methodToString(request->method);

    int key_len = snprintf(out, out_size, "%s|%s|%s", method, host, request->requestURI);

    if (key_len < 0 || (size_t)key_len >= out_size) {
        return -1;
    }

    return 0;
}

int http_request_is_cacheable(const Request *request) {
    if (!request) return 0;
    return request->method == METHOD_GET || request->method == METHOD_HEAD;

}

