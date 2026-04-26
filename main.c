#include "HTTP/requestParser.h"
#include "HTTP/processRequest.h"

int main(){

    // Variables para probar
    RequestLine *req = createRequest();
    HTTP_Response *res = createHTTPResponse();

char request[8192];  // buffer más grande para request line + headers
size_t len = 0;
char line[2048];

printf("Ingresa la request:\n");

    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("Error leyendo entrada\n");
            return -1;
        }
        
        // Quitar el \n que agrega fgets
        size_t lineLen = strlen(line);
        if (lineLen > 0 && line[lineLen - 1] == '\n') {
            line[lineLen - 1] = '\0';
            lineLen--;
        }
        // Por si viene con \r\n (terminales Windows)
        if (lineLen > 0 && line[lineLen - 1] == '\r') {
            line[lineLen - 1] = '\0';
            lineLen--;
        }
        
        // Verificar que cabe en el buffer (línea + \r\n + null terminator)
        if (len + lineLen + 3 > sizeof(request)) {
            printf("Error: request demasiado grande\n");
            return -1;
        }
        
        // Si la línea está vacía, terminamos los headers
        if (lineLen == 0) {
            // Agregar el \r\n final que separa headers del body
            memcpy(request + len, "\r\n", 2);
            len += 2;
            request[len] = '\0';
            break;
        }
        
        // Copiar la línea + \r\n al buffer principal
        memcpy(request + len, line, lineLen);
        len += lineLen;
        memcpy(request + len, "\r\n", 2);
        len += 2;
        request[len] = '\0';
    }

    // ---------------------------------

    size_t position;
    int parseReqLine = parseRequestLine(request, len, req , &position);

    // Si esto pasa, fue una bad request
    if(parseReqLine == -1){
        printf("Pailas, se nos jodio el parser\n");
        res->status = STATUS_400;

        printRequest(req);
        printResponse(res);

        freeResponse(res);
        freeRequest(req);
        return 0;
    }

    int parseHead = parseHeaders(request, len, req , &position);

    if(parseHead == -1){
        printf("Pailas, se nos jodio el parser\n");
        res->status = STATUS_400;

        printResponse(res);

        freeResponse(res);
        freeRequest(req);
        return 0;
    }

    res->status = processRequest(req , res);
    
    printRequest(req);
    printResponse(res);

    freeResponse(res);
    freeRequest(req);

    return 0;
}