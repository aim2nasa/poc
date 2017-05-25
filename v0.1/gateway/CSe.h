#ifndef __CSE_H__
#define __CSE_H__

#include "protocol.h"
#include "pkcs11.h"

class CSe{
public:
	CSe() :h_(CK_INVALID_HANDLE){}

	unsigned char serialNo[SERIAL_NO_SIZE];
	unsigned int cid_;
	CK_OBJECT_HANDLE h_;
};

#endif