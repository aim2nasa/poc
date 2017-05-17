#ifndef __HELPER_H__
#define __HELPER_H__

#include "cryptoki.h"
class CToken;

int prepareSession(CToken &token, const char *label, const char *soPin, const char *userPin);
int gatewayKey(CToken &token, CK_ULONG keySize, CK_OBJECT_HANDLE &hGw);

#endif