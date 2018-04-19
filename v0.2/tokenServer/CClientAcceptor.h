#ifndef __CCLIENT_ACCEPTOR_H__
#define __CCLIENT_ACCEPTOR_H__

#include "Stream_Handler.h"

class CHsmProxy;

class CClientAcceptor : public ACE_Acceptor<Stream_Handler, ACE_SOCK_ACCEPTOR>
{
public:
#ifdef USE_SOFTHSM
	CClientAcceptor() :pHsm_(NULL), hTagKey_(0), hSeKey_(0){}
	CHsmProxy *pHsm_;
	unsigned long hTagKey_, hSeKey_;
#endif
};

#endif
