#ifndef __CCLIENT_ACCEPTOR_H__
#define __CCLIENT_ACCEPTOR_H__

#include "Stream_Handler.h"

#ifdef USE_SOFTHSM
class CHsmProxy;
#elif defined(USE_OPTEE)
#include <okey.h>
#endif

class CClientAcceptor : public ACE_Acceptor<Stream_Handler, ACE_SOCK_ACCEPTOR>
{
public:
#ifdef USE_SOFTHSM
	CClientAcceptor() :pHsm_(NULL), hTagKey_(0), hSeKey_(0){}
	CHsmProxy *pHsm_;
	unsigned long hTagKey_, hSeKey_;
#elif defined(USE_OPTEE)
	CClientAcceptor() :pO_(NULL), tagKey_(0), seKey_(0){}
	okey *pO_;
	uint32_t tagKey_, seKey_;
#endif
};

#endif
