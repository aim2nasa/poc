#ifndef __CGROUP_H__
#define __CGROUP_H__

#include <list>

class CGroup{
public:
	std::string groupName_;
	std::list<unsigned int> cidList_;
};

#endif