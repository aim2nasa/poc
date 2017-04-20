#include "library.h"
#include <windows.h>
#include <iostream>

using namespace std;

int loadLib(void **module, CK_FUNCTION_LIST_PTR *p11)
{
	*module = LoadLibraryA(DEFAULT_PKCS11_LIB);
	if (*module == NULL) {
		cout << "ERROR: LoadLibraryA failed : " << GetLastError() << endl;
		return -1;
	}

	CK_C_GetFunctionList pGetFunctionList = (CK_C_GetFunctionList)GetProcAddress((HMODULE)*module, "C_GetFunctionList");
	if (pGetFunctionList == NULL) {
		cout << "ERROR: getProcAddress failed : " << GetLastError() << endl;
		unloadLib(*module);
		return -1;
	}
	cout << "0x" << hex << pGetFunctionList << " C_GetFunctionList retrived" << endl;

	// Load the function list
	(*pGetFunctionList)(p11);

	// Initialize the library
	if ((*p11)->C_Initialize(NULL_PTR) != CKR_OK) {
		cout << "ERROR: Could not initialize the library" << endl;
		unloadLib(*module);
		return -1;
	}
	return 0;
}

void unloadLib(void *module)
{
	FreeLibrary((HMODULE)module);
}