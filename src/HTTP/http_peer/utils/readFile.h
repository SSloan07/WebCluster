#ifndef READ_FILE_H
#define READ_FILE_H

#include <stdlib.h>

char *readFile(const char *filePath , size_t *outSize);
const char *getContentType(const char *filePath);
int fileExist(const char *filePath);

#endif