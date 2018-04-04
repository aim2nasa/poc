#ifndef __CGROUP_H__
#define __CGROUP_H__

#ifndef _WIN32
#include <string>
#endif
#include <list>
#include "CSe.h"

class CGroup{
public:
	CGroup();
	virtual ~CGroup();

	void tagKey(CK_BYTE_PTR tKey, CK_ULONG size);
	CK_BYTE_PTR tagKey();

	std::string groupName_;
	std::list<CSe> seList_;
	CK_OBJECT_HANDLE hGroup_;
	CK_OBJECT_HANDLE hTag_;

private:
	CK_BYTE tagKey_[AES_KEY_SIZE];
};

#endif