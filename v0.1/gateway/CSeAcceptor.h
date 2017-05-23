#ifndef __CSEACCEPTOR_H__
#define __CSEACCEPTOR_H__

#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>
#include "StreamHandler.h"

//SE로 부터 오는 접속을 처리하는 Acceptor클래스
class CSeAcceptor : public ACE_Acceptor<StreamHandler, ACE_SOCK_ACCEPTOR>
{

};

#endif