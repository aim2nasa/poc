#include "util.h"

std::ostream& operator<< (std::ostream& os, const HelloWorldData::Msg& m)
{
  os << "[ userID=" << m.userID()
     << ",message=" << m.message()
     << "]";
  return os;
}

