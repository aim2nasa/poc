#ifndef __CSEACCEPTOR_H__
#define __CSEACCEPTOR_H__

#include <ace/Acceptor.h>
#ifdef USE_SSL
#include <ace/SSL/SSL_SOCK_Acceptor.h>
#else
#include <ace/SOCK_Acceptor.h>
#endif
#include "StreamHandler.h"

//SE�� ���� ���� ������ ó���ϴ� AcceptorŬ����
#ifdef USE_SSL
class CSeAcceptor : public ACE_Acceptor<StreamHandler, ACE_SSL_SOCK_Acceptor>
#else
class CSeAcceptor : public ACE_Acceptor<StreamHandler, ACE_SOCK_ACCEPTOR>
#endif
{

};

#endif