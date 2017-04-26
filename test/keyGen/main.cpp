#include <iostream>
#include "library.h"
#include "helper.h"

using namespace std;

int main(int argc, const char* argv[])
{
	if (argc < 4) {
		cout << "usage: testpkcs11 <soPin> <label> <userPin>" << endl;
		return -1;
	}

	void* module;
	CK_FUNCTION_LIST_PTR p11 = NULL;
	if (loadLib(&module, &p11) == -1) {
		cout << "ERROR: loadLib" << endl;
		return -1;
	}
	cout << "loadLib ok" << endl;

	CK_SESSION_HANDLE hSession;
	prepare(p11, argv[1],argv[2],argv[3],hSession);

	//GwKey(AES키를 생성)
	CK_OBJECT_HANDLE hGw = CK_INVALID_HANDLE;
	createAesKey(hSession, hGw, 32, "gw");

	CK_BYTE salt[32];

	//Derive G1
	CK_OBJECT_HANDLE hG1 = CK_INVALID_HANDLE;
	memInit(salt, sizeof(salt), "Unique Name of G1");
	deriveGroup(hSession, hG1, hGw, salt, sizeof(salt), "G1");

	//Derive T1 from G1
	CK_OBJECT_HANDLE hT1 = CK_INVALID_HANDLE;
	deriveTagFromGroup(hSession, hT1, hG1, "T1");

	//Derive G11 from G1
	CK_OBJECT_HANDLE hG11 = CK_INVALID_HANDLE;
	memInit(salt, sizeof(salt), "Serial Number of G11");
	deriveGroup(hSession, hG11, hG1, salt, sizeof(salt), "G11");

	//Derive G12 from G1
	CK_OBJECT_HANDLE hG12 = CK_INVALID_HANDLE;
	memInit(salt, sizeof(salt), "Serial Number of G12");
	deriveGroup(hSession, hG12, hG1, salt, sizeof(salt), "G12");

	//Derive G13 from G1
	CK_OBJECT_HANDLE hG13 = CK_INVALID_HANDLE;
	memInit(salt, sizeof(salt), "Serial Number of G13");
	deriveGroup(hSession, hG13, hG1, salt, sizeof(salt), "G13");

	unloadLib(module);
	cout << "end" << endl;
	return 0;
}