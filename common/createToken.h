#include <pkcs11.h>

int createToken(CK_FUNCTION_LIST_PTR p11, CK_SESSION_HANDLE *pSession, char *soPin, char *label, char *userPin);