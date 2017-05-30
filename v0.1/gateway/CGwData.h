#ifndef __CGWDATA_H__
#define __CGWDATA_H__

#include <map>
#include "StreamHandler.h"
#include "pkcs11.h"
#include "CGroup.h"

class CToken;
class CCtrlProxy;

class CGwData{
private:
	CGwData();

public:
	static CGwData* getInstance();
	static void delInstance();

	std::map<CID, StreamHandler*> con_;
	CK_OBJECT_HANDLE hGw_;
	std::list<CGroup> groupList_;
	CToken *token_;
	CCtrlProxy *ctrlProxy_;
private:
	static CGwData *gwData_;
};

#endif