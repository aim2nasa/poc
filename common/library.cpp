#include "library.h"
#include <windows.h>
#include <iostream>
#include <assert.h>

using namespace std;

int loadLib(void **module, CK_FUNCTION_LIST_PTR *p11)
{
	int nRtn = loadLibOnly(module, p11);
	switch (nRtn){
	case -1:
		cout << "ERROR: LoadLibraryA failed : " << GetLastError() << endl;
		return -1;
	case -2:
		cout << "ERROR: getProcAddress failed : " << GetLastError() << endl;
		unloadLib(*module);
		break;
	default:
		assert(nRtn == 0);
	}

	// Initialize the library
	if ((*p11)->C_Initialize(NULL_PTR) != CKR_OK) {
		cout << "ERROR: Could not initialize the library" << endl;
		unloadLib(*module);
		return -1;
	}
	return 0;
}

int loadLibOnly(void **module, CK_FUNCTION_LIST_PTR *p11)
{
	*module = LoadLibraryA(DEFAULT_PKCS11_LIB);
	if (*module == NULL)
		return -1;

	CK_C_GetFunctionList pGetFunctionList = (CK_C_GetFunctionList)GetProcAddress((HMODULE)*module, "C_GetFunctionList");
	if (pGetFunctionList == NULL) {
		unloadLib(*module);
		return -2;
	}

	(*pGetFunctionList)(p11);
	return 0;
}

void unloadLib(void *module)
{
	FreeLibrary((HMODULE)module);
}