#ifndef __LIBRARY_H__
#define __LIBRARY_H__

#include "cryptoki.h"

#define DEFAULT_PKCS11_LIB "softhsm2.dll"

int loadLib(void **module, CK_FUNCTION_LIST_PTR *p11);
void unloadLib(void *module);

#endif