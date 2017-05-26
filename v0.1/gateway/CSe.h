#ifndef __CSE_H__
#define __CSE_H__

#include "protocol.h"
#include "pkcs11.h"

class CSe{
public:
	CSe();

	unsigned int cid_;
	CK_OBJECT_HANDLE h_;
};

#endif