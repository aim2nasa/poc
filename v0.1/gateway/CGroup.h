#ifndef __CGROUP_H__
#define __CGROUP_H__

#include <list>

class CGroup{
public:
	CGroup() :hGroup_(CK_INVALID_HANDLE), hTag_(CK_INVALID_HANDLE){}

	std::string groupName_;
	std::list<unsigned int> cidList_;
	CK_OBJECT_HANDLE hGroup_;
	CK_OBJECT_HANDLE hTag_;
};

#endif