#include "stdafx.h"
#include "CConTable.h"

CConTable::~CConTable()
{
	clear();
}

void CConTable::insert(unsigned int cid, char *serialNo, unsigned int serialNoSize)
{
	char *pSn = new char[serialNoSize];
	table_.insert( std::pair<unsigned int, char*>(cid, pSn));
}

void CConTable::clear()
{
	std::map<unsigned int, char*>::iterator it;
	for (it = table_.begin(); it != table_.end(); it++) delete[] it->second;
}