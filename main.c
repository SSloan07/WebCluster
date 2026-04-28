#include "HTTP/requestParser.h"
#include "HTTP/processRequest.h"

int main(){

    // Variables para probar
    Request *req = createRequest();
    HTTP_Response *res = createHTTPResponse();

char request[1048576];
size_t len = 0;
size_t contentLength = 0;
char line[1024];

while (1) {
    if (fgets(line, sizeof(line), stdin) == NULL) {
        break;  
    }
    
    
    size_t lineLen = strlen(line);
    if (lineLen > 0 && line[lineLen - 1] == '\n') {
        line[lineLen - 1] = '\0';
        lineLen--;
    }
    if (lineLen > 0 && line[lineLen - 1] == '\r') {
        line[lineLen - 1] = '\0';
        lineLen--;
    }
    
    
    if (len + lineLen + 3 > sizeof(request)) {
        printf("Error: request excede el buffer\n");
        return -1;
    }
    
    
    if (strncasecmp(line, "Content-Length:", 15) == 0) {
        const char *value = line + 15;
        while (*value == ' ' || *value == '\t') value++;
        contentLength = (size_t)strtoul(value, NULL, 10);
    }
    
    
    if (lineLen == 0) {
        memcpy(request + len, "\r\n", 2);
        len += 2;
        
        
        if (contentLength > 0) {
            if (len + contentLength > sizeof(request)) {
                printf("Error: body excede el buffer\n");
                return -1;
            }
            
            size_t totalRead = 0;
            while (totalRead < contentLength) {
                size_t bytesRead = fread(request + len + totalRead, 1,
                                         contentLength - totalRead, stdin);
                if (bytesRead == 0) {
                    
                    break;
                }
                totalRead += bytesRead;
            }
            len += totalRead;  
        }
        break;
    }
    
    
    memcpy(request + len, line, lineLen);
    len += lineLen;
    memcpy(request + len, "\r\n", 2);
    len += 2;
}
    // ---------------------------------

    size_t position = 0;
    int parseReqLine = parseRequestLine(request, len, req , &position);

    // Si esto pasa, fue una bad request
    if(parseReqLine == -1){
        printf("Pailas, se nos jodio el parser del req line\n");
        res->status = STATUS_400;

        printResponse(res);

        freeResponse(res);
        freeRequest(req);
        return 0;
    }

    int parseHead = parseHeaders(request, len, req , &position);

    if(parseHead == -1){
        printf("Pailas, se nos jodio el parser del header\n");
        res->status = STATUS_400;

        printResponse(res);

        freeResponse(res);
        freeRequest(req);
        return 0;
    }

    if(req->method == METHOD_POST) {

        int parseBod = parseBody(request , len , req , &position);

        if(parseBod == -1){
            printf("Pailas, se nos jodio el parser del body\n");
            res->status = STATUS_400;

            printResponse(res);

            freeResponse(res);
            freeRequest(req);
            return 0;
        }

    }

    res->status = processRequest(req , res);
    
    printRequest(req);
    printResponse(res);

    freeResponse(res);
    freeRequest(req);

    return 0;
}