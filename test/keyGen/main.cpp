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

	//GwKey(AESŰ�� ����)
	CK_OBJECT_HANDLE hGw = CK_INVALID_HANDLE;
	createAesKey(hSession, hGw, 32, "gw");

	CK_BYTE salt[32];

	//Derive G1 -----------------------------------------------------------------
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

	//Derive G2 -----------------------------------------------------------------
	CK_OBJECT_HANDLE hG2 = CK_INVALID_HANDLE;
	memInit(salt, sizeof(salt), "Unique Name of G2");
	deriveGroup(hSession, hG2, hGw, salt, sizeof(salt), "G2");

	//Derive T2 from G2
	CK_OBJECT_HANDLE hT2 = CK_INVALID_HANDLE;
	deriveTagFromGroup(hSession, hT2, hG2, "T2");

	//Derive G21 from G2
	CK_OBJECT_HANDLE hG21 = CK_INVALID_HANDLE;
	memInit(salt, sizeof(salt), "Serial Number of G21");
	deriveGroup(hSession, hG21, hG2, salt, sizeof(salt), "G21");

	//Derive G22 from G2
	CK_OBJECT_HANDLE hG22 = CK_INVALID_HANDLE;
	memInit(salt, sizeof(salt), "Serial Number of G22");
	deriveGroup(hSession, hG22, hG2, salt, sizeof(salt), "G22");

	//AES ECB encoding/decoding test
	const int blockSize(0x10);
	const int NumBlock(10);
	CK_BYTE data[blockSize*NumBlock] = { "Billy Elliot the musical. What a fascinating experince! Love it" };

	aesEcbEncDec(hSession, hT1, blockSize, data, sizeof(data));

	unloadLib(module);
	cout << "end" << endl;
	return 0;
}