#include "gmock/gmock.h"
#include "mock/mock-ISession.h"

class Session{
public:
    Session(ISession *p):sess_(p){}

    int init(const char *ip,ushort port){ return sess_->init(ip,port); }
    int close(){ return sess_->close(); }

private:
    ISession *sess_;
};

TEST(Session, openClose)
{
    MockSession ms;
    EXPECT_CALL(ms,init(NULL,0))
        .WillRepeatedly(testing::Return(0));

    EXPECT_CALL(ms,close())
        .WillRepeatedly(testing::Return(0));

    Session s(&ms);
    EXPECT_EQ(s.init(0,0),0);
    EXPECT_EQ(s.close(),0);
}
