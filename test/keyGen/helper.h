#include "library.h"

void memInit(CK_BYTE_PTR data, CK_ULONG dataSize, const char *name);
int prepare(const char *confPath, CK_FUNCTION_LIST_PTR p11, const char *soPin, const char *label, const char *userPin, CK_SESSION_HANDLE &hSession);
int createAesKey(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE &hKey, CK_ULONG bytes, const char *gw);
int deriveGroup(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE &hGroup, CK_OBJECT_HANDLE hParent, CK_BYTE_PTR data, CK_ULONG dataSize, const char *group);
int deriveTagFromGroup(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE &hTag, CK_OBJECT_HANDLE hGroup, const char *tag);
int aesDerive(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey, CK_OBJECT_HANDLE &hDerive, CK_MECHANISM_TYPE mechType, CK_BYTE *data, CK_LONG dataSize, CK_CHAR_PTR iv = NULL);
CK_RV printKey(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey);
void aesEcbEncDec(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey, const size_t blockSize, CK_BYTE_PTR data, CK_LONG dataSize);
int setenv(const char *name, const char *value, int overwrite);