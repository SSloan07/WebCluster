#include "HttpParser.h"
#include "http_peer/processRequest.h"
#include "http_peer/utils/readFile.h"

/*
 * Public facade of the HTTP module.
 * The parsing functions still live in HttpParser_peer.c.
 * This unit exposes wrappers so the rest of the project depends on
 * src/HTTP rather than directly on http_peer.
 */

HTTP_Status processRequest(Request *req, HTTP_Response *res) {
    return http_peer_process_request(req, res);
}

void http_set_document_root(const char *root_path) {
    setDocumentRoot(root_path);
}
