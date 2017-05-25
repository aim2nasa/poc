#ifndef __HELPER_H__
#define __HELPER_H__

#include "cryptoki.h"
class CToken;

int prepareSession(CToken &token, const char *label, const char *soPin, const char *userPin);
int gatewayKey(CToken &token, CK_ULONG keySize, CK_OBJECT_HANDLE &hGw);
int aesDerive(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey, CK_OBJECT_HANDLE &hDerive, CK_MECHANISM_TYPE mechType, CK_BYTE *data, CK_LONG dataSize, CK_CHAR_PTR iv = NULL);
int deriveGroup(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE &hGroup, CK_OBJECT_HANDLE hParent, CK_BYTE_PTR data, CK_ULONG dataSize);

#endif