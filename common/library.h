#ifndef __LIBRARY_H__
#define __LIBRARY_H__

#include "cryptoki.h"

CK_C_GetFunctionList loadLibrary(char* module, void** moduleHandle, char **pErrMsg);
void unloadLibrary(void* moduleHandle);

#endif