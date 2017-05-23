#ifndef __CGWDATA_H__
#define __CGWDATA_H__

#include <map>
#include "StreamHandler.h"

class CGwData{
private:
	CGwData();

public:
	static CGwData* getInstance();
	static void delInstance();

	std::map<CID, StreamHandler*> con_;

private:
	static CGwData *gwData_;
};

#endif