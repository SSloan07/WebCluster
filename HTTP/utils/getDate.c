#include "getDate.h"

#include <time.h>

void getDate(char *buffer , size_t bufferSize){
    time_t now = time(NULL);
    struct tm *gtm = gmtime(&now);
    strftime(buffer , bufferSize , "%a, %d %b %Y %H:%M:%S GMT" , gtm);
}