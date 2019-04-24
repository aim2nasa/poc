#ifndef __MOCK_ISESSION_H__
#define __MOCK_ISESSION_H__

#include "gmock/gmock.h"
#include "../src/ISession.h"

class MockSession : public ISession{
public:
    MOCK_METHOD2(init,int(const char *ip,ushort port));
    MOCK_METHOD0(close,int());
};

#endif
