#ifndef __CTOKEN_H__
#define __CTOKEN_H__

#include "cryptoki.h"

#define MAX_ERR_MSG 256

class CToken{
public:
	CToken();
	virtual ~CToken();

	int initialize();
	int slotCount(CK_ULONG &ulSlotCount);

	char _message[MAX_ERR_MSG];

protected:
	void *_module;
	CK_FUNCTION_LIST_PTR _p11;
};

#endif