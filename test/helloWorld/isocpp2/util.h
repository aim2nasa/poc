#ifndef __UTIL_H__
#define __UTIL_H__

#include <iostream>
#include <gen/HelloWorldData_DCPS.hpp>

std::ostream& operator<< (std::ostream& os, const HelloWorldData::Msg& m);

#endif
