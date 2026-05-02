#ifndef READ_FILE_H
#define READ_FILE_H

#include <stdlib.h>

void setDocumentRoot(const char *rootPath);
char *readFile(const char *filePath , size_t *outSize);
int buildDocumentPath(const char *filePath, char *outPath, size_t outSize);
const char *getContentType(const char *filePath);

#endif
