#ifndef __HELPER_H__
#define __HELPER_H__

#include "cryptoki.h"
class CToken;

int gatewayKey(CToken &token, CK_ULONG keySize, CK_OBJECT_HANDLE &hGw);
int aesDerive(CToken &token, CK_OBJECT_HANDLE hKey, CK_OBJECT_HANDLE &hDerive, CK_MECHANISM_TYPE mechType, CK_BYTE *data, CK_LONG dataSize, CK_CHAR_PTR iv = NULL);
int deriveGroup(CToken &token, CK_OBJECT_HANDLE &hGroup, CK_OBJECT_HANDLE hParent, CK_BYTE_PTR data, CK_ULONG dataSize);
int deriveTagFromGroup(CToken &token, CK_OBJECT_HANDLE &hTag, CK_OBJECT_HANDLE hGroup);
int showKey(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey, const char *name);
int getKey(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey, CK_BYTE_PTR key, CK_ULONG keySize);
void displayKey(CK_BYTE_PTR key, CK_ULONG keySize, const char *name);

#endif