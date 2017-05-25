#ifndef __CONTABLE_H__
#define __CONTABLE_H__

#include <iostream>
#include <map>

class CConTable{
public:
	virtual ~CConTable();

	void insert(unsigned int cid, char *serialNo, unsigned int serialNoSize);
	void clear();

	std::map<unsigned int, char*> table_;	//char*는 동적할당된 메모리이다.
};

#endif