#ifndef ENUM_TO_STRING
#define ENUM_TO_STRING
#include "../structs/requestLine.h"
#include "../structs/response.h"

const char *methodToString(HTTP_Method method);
const char *versionToString(HTTP_Version version);
const char *statusToString(HTTP_Status status);
const char *statusToReasonPhrase(HTTP_Status status);

#endif