#include <iostream>
#include "library.h"

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

	const char *soPin = argv[1];
	const char *label = argv[2];
	const char *userPin = argv[3];

	//��ū�� �����Ѵ� (���� ���� ����)
	CK_SESSION_HANDLE hSession;
	CK_ULONG ulSlotCount;
	CK_RV rv = p11->C_GetSlotList(CK_FALSE, NULL_PTR, &ulSlotCount);
	if (rv != CKR_OK) {
		cout << "ERROR: Couldn't get the number of slots: 0x" << hex << rv << endl;
		return -1;
	}
	cout << "number of slots:" << ulSlotCount << endl;

	CK_SLOT_ID slotID = ulSlotCount - 1; //����Ʈ�� ���� �Ѱ��� ī��Ʈ�� �����Ѵ�. ������ �ϳ��� �������� ī��Ʈ�� 1�� �����Ƿ�
	cout << "SlotID : 0x" << hex << slotID << dec << " (" << slotID << ")" << endl;

	CK_UTF8CHAR paddedLabel[32];
	memset(paddedLabel, ' ', sizeof(paddedLabel));
	memcpy(paddedLabel, label, strlen(label));

	rv = p11->C_InitToken(slotID, (CK_UTF8CHAR_PTR)soPin, (CK_ULONG)strlen(soPin), paddedLabel);
	if (rv != CKR_OK) {
		cout << "ERROR: C_InitToken: 0x" << hex << rv << endl;
		return -1;
	}

	rv = p11->C_OpenSession(slotID, CKF_SERIAL_SESSION | CKF_RW_SESSION, NULL_PTR, NULL_PTR, &hSession);
	if (rv != CKR_OK) {
		cout << "ERROR: C_OpenSession: 0x" << hex << rv << endl;
		return -1;
	}

	rv = p11->C_Login(hSession, CKU_SO, (CK_UTF8CHAR_PTR)soPin, (CK_ULONG)strlen(soPin));
	if (rv != CKR_OK) {
		cout << "ERROR: C_Login: 0x" << hex << rv << endl;
		return -1;
	}

	rv = p11->C_InitPIN(hSession, (CK_UTF8CHAR_PTR)userPin, (CK_ULONG)strlen(userPin));
	if (rv != CKR_OK) {
		cout << "ERROR: C_InitPIN: 0x" << hex << rv << endl;
		return -1;
	}

	unloadLib(module);
	cout << "end" << endl;
	return 0;
}