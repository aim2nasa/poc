#ifndef __CSE_H__
#define __CSE_H__

#ifndef _WIN32
#include "cryptoki.h"
#endif
#include "protocol.h"
#include "pkcs11.h"

class CSe{
public:
	CSe();

	void tagKey(CK_BYTE_PTR tKey, CK_ULONG size);
	CK_BYTE_PTR tagKey();
	void seKey(CK_BYTE_PTR seKey, CK_ULONG size);
	CK_BYTE_PTR seKey();

	unsigned int cid_;
	CK_OBJECT_HANDLE h_;

private:
	CK_BYTE tagKey_[AES_KEY_SIZE];
	CK_BYTE seKey_[AES_KEY_SIZE];
};

#endif