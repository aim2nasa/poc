#ifndef __CSEACCEPTOR_H__
#define __CSEACCEPTOR_H__

#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>
#include "StreamHandler.h"

//SE�� ���� ���� ������ ó���ϴ� AcceptorŬ����
class CSeAcceptor : public ACE_Acceptor<StreamHandler, ACE_SOCK_ACCEPTOR>
{

};

#endif