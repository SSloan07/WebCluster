#include "HTTP/requestParser.h"
#include "HTTP/processRequest.h"

int main(){

    // Variables para probar
    RequestLine *req = createRequestLine();
    HTTP_Response *res = createHTTPResponse();

    char request[2048];  // buffer fijo, suficiente para un request line

    printf("Ingresa el request line: ");
    if (fgets(request, sizeof(request), stdin) == NULL) {
        printf("Error leyendo entrada\n");
        return -1;
    }

    // fgets incluye el \n al final, lo quitamos y lo reemplazamos por \r\n
    size_t len = strlen(request);
    if (len > 0 && request[len - 1] == '\n') {
        request[len - 1] = '\0';
        len--;
    }

    // Agregar el \r\n que tu parser espera
    strcat(request, "\r\n");
    len = strlen(request);

    // ---------------------------------

    int parse = parseRequestLine(request, len, req);

    // Si esto pasa, fue una bad request
    if(parse == -1){
        printf("Pailas, se nos jodio el parser\n");
        res->status = STATUS_400;

        printRequestLine(req);
        printResponse(res);

        free(req);
        free(res);
        return 0;
    }

    res->status = processRequest(req , res);
    
    // Si esto pasa, lo mas probable es que la version o el me
    

    printRequestLine(req);
    printResponse(res);

    free(req);
    free(res);

    return 0;
}