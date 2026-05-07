#include "HttpParser.h"
#include "http_peer/processRequest.h"
#include "http_peer/utils/readFile.h"

/*
 * Fachada publica del modulo HTTP.
 * Las funciones de parseo siguen viviendo en HttpParser_peer.c.
 * Esta unidad expone wrappers para que el resto del proyecto dependa
 * de src/HTTP y no directamente de http_peer.
 */

HTTP_Status processRequest(Request *req, HTTP_Response *res) {
    return http_peer_process_request(req, res);
}

void http_set_document_root(const char *root_path) {
    setDocumentRoot(root_path);
}
