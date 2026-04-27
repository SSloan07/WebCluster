#include "requestParser.h"
#include "utils/getEnumRequestLine.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_URI_LENGTH 2048

int parseRequestLine(const char *rawRequestLine , size_t rawLength, RequestLine *req){

    int count = 0;
    size_t lastLetter = 0;

    // Parsear todos los espacios en blanco y mirar que espacio corresponde a que parte del request
    for(size_t i = 0 ; i < rawLength ; i++){
        
        // Movemos el puntero i+1 veces (Ya que i es el espacio que separa entre expresiones)
        // Al hacer rawRequestLine + lastLetter estamos moviendo lastLetter veces el puntero para que apunte a la ultima letra encontrada
        if(rawRequestLine[i] == ' ' && count == 0){

            size_t length = i - lastLetter;
            char tempMethod[16];

            if(length >= sizeof(tempMethod) || length == 0) return -1;

            memcpy(tempMethod, rawRequestLine + lastLetter , length);
            tempMethod[length] = '\0';
            req->method = getHTTPMethod(tempMethod);
            
            lastLetter = i+1;
            count++;
        }
        
        else if(rawRequestLine[i] == ' ' && count == 1){

            size_t length = i - lastLetter;

            if (length == 0 || length > MAX_URI_LENGTH) return -1;

            req->requestURI = malloc(length + 1);
            memcpy(req->requestURI , rawRequestLine + lastLetter , length);
            req->requestURI[length] = '\0';

            lastLetter = i+1;
            count++;
        }

        else if(rawRequestLine[i] == '\r' && count == 2){

            if(i + 1 >= rawLength || rawRequestLine[i+1] != '\n' ) return -1;

            size_t length = i - lastLetter;
            char tempVersion[32];
            
            if(length >= sizeof(tempVersion) || length == 0 ) return -1;

            memcpy(tempVersion , rawRequestLine + lastLetter , length);
            tempVersion[length] = '\0';
            req->httpVersion = getHTTPVersion(tempVersion);

            lastLetter = i+1;
            count++;
        }
    }

    if(count != 3){
        printf("No se encontraron los 3 argumentos necesarios del request line\n");
        return -1;
    }

    return 0;
}