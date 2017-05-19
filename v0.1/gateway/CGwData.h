#ifndef __CGWDATA_H__
#define __CGWDATA_H__

#include <map>
#include "StreamHandler.h"

class CGwData{
public:
	std::map<CID, StreamHandler*> con_;
};

#endif