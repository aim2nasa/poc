#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <ace/Message_Block.h>
#include <ace/INET_Addr.h>
#include <ace/Svc_Handler.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>
#include <ace/Synch_Traits.h>
#include <ace/Reactor.h>
#include <ace/Acceptor.h>
#include <ace/Reactor_Notification_Strategy.h>
#include "CClientAcceptor.h"

#include <common.h>
#ifdef USE_SOFTHSM
#include "CHsmProxy.h"
#elif USE_OPTEE
#include <okey.h>
#endif

#define SERVER_PORT 9870

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
#ifdef USE_SOFTHSM
	ACE_DEBUG((LM_INFO, "configured to use softhsm\n"));
	if (argc<3) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("usage:tokenServer <port> <userPin>\n")));
		ACE_ERROR((LM_ERROR, ACE_TEXT("      port:set 0 for defalut port(9870)\n")));
		ACE_RETURN(-1);
	}
#elif defined(USE_OPTEE)
	ACE_DEBUG((LM_INFO, "configured to use optee\n"));
	if (argc<2) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("usage:tokenServer <port>\n")));
		ACE_ERROR((LM_ERROR, ACE_TEXT("      port:set 0 for defalut port(9870)\n")));
		ACE_RETURN(-1);
	}
#else
	ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Error predefine missing, define USE_SOFTHSM or USE_OPTEE\n")), -1);
#endif
	u_short server_port;
	ACE_OS::atoi(argv[1]) == 0 ? server_port = (u_short)SERVER_PORT : server_port = ACE_OS::atoi(argv[1]);
	ACE_DEBUG((LM_INFO, "(%t) gateway start at port:%u\n", server_port));

	ACE_INET_Addr listen;
	listen.set(server_port);

#ifdef USE_SOFTHSM
	CHsmProxy hsm;
#ifndef _WIN32
        hsm.setenv("SOFTHSM2_CONF", "./softhsm2-linux.conf", 1);
#else
        hsm.setenv("SOFTHSM2_CONF", ".\\softhsm2.conf", 1);
#endif
	int nRtn;
	if ((nRtn = hsm.init(argv[2])) != 0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("HSM init failure(userPin:%s):%d (%s)"), argv[2], nRtn, hsm.message_), -1);

	ACE_DEBUG((LM_INFO, "(%t) SlotID:%u\n", hsm.slotID()));

	unsigned long hTagKey, hSeKey;
	if (hsm.findKey(TAG_KEY_LABEL, sizeof(TAG_KEY_LABEL)-1, hTagKey) != 0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("Tag key(%s) not found\n"), TAG_KEY_LABEL), -1);

	ACE_ASSERT(hTagKey != 0);
	ACE_DEBUG((LM_INFO, "(%t) Tag key(%d) retrieved\n", hTagKey));

	if (hsm.findKey(SE_KEY_LABEL, sizeof(SE_KEY_LABEL)-1, hSeKey) != 0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("SE key(%s) not found\n"), SE_KEY_LABEL), -1);

	ACE_ASSERT(hSeKey != 0);
	ACE_DEBUG((LM_INFO, "(%t) SE key(%d) retrieved\n", hSeKey));
#elif defined(USE_OPTEE)
	okey o;
	TEEC_Result res = initializeContext(NULL,&o);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("initializeContext failed 0x%x"),res), -1);

	res = openSession(&o,TEEC_LOGIN_PUBLIC,NULL,NULL);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("openSession failed 0x%x"),res), -1);

	uint32_t tagKey,seKey;
	res = keyOpen(&o,TEE_STORAGE_PRIVATE,TAG_KEY_LABEL,&tagKey);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("keyOpen(%s) failed 0x%x"),TAG_KEY_LABEL,res), -1);
	ACE_DEBUG((LM_INFO, "(%t) Tag key(0x%x) retrieved\n", tagKey));

	res = keyOpen(&o,TEE_STORAGE_PRIVATE,SE_KEY_LABEL,&seKey);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("keyOpen(%s) failed 0x%x"),SE_KEY_LABEL,res), -1);
	ACE_DEBUG((LM_INFO, "(%t) SE key(0x%x) retrieved\n", seKey));

	size_t keySize = 256; //TODO hardcode for the time being. need to be configurable and monitored through API in libokey

	OperationHandle encOp;
	res = keyAllocOper(&o,true,keySize,&encOp);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("keyAllocOper(encode) failed 0x%x"),res), -1);
	ACE_DEBUG((LM_INFO, "(%t) encode operation handle:0x%x\n", encOp));

	OperationHandle decOp;
	res = keyAllocOper(&o,false,keySize,&decOp);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("keyAllocOper(decode) failed 0x%x"),res), -1);
	ACE_DEBUG((LM_INFO, "(%t) decode operation handle:0x%x\n", decOp));

	//inject key for operations
	res = keySetkeyOper(&o,encOp,tagKey);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("encode keySetkeyOper(0x%x) failed 0x%x"),tagKey,res), -1);
	ACE_DEBUG((LM_INFO, "(%t) setkey(0x%x) for encode operation(0x%x)\n", tagKey,encOp));

	res = keySetkeyOper(&o,decOp,tagKey);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("decode keySetkeyOper(0x%x) failed 0x%x"),tagKey,res), -1);
	ACE_DEBUG((LM_INFO, "(%t) setkey(0x%x) for decode operation(0x%x)\n", tagKey,decOp));

	uint8_t shMemFactor = 1;
	res = cipherInit(&o,encOp,shMemFactor);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("encode cipherInit(0x%x,%d) failed 0x%x"),encOp,shMemFactor,res), -1);
	ACE_DEBUG((LM_INFO, "(%t) encode cipher initialized(0x%x) with shMemFactor=%d\n", encOp,shMemFactor));

	res = cipherInit(&o,decOp,shMemFactor);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("decode cipherInit(0x%x,%d) failed 0x%x"),decOp,shMemFactor,res), -1);
	ACE_DEBUG((LM_INFO, "(%t) decode cipher initialized(0x%x) with shMemFactor=%d\n", decOp,shMemFactor));
#endif //USE_SOFTHSM

	CClientAcceptor acceptor;
#ifdef USE_SOFTHSM
	acceptor.pHsm_ = &hsm;
	acceptor.hTagKey_ = hTagKey;
	acceptor.hSeKey_ = hSeKey;
#elif defined(USE_OPTEE)
	acceptor.pO_ = &o;
	acceptor.tagKey_ = tagKey;
	acceptor.seKey_ = seKey;
#endif
	acceptor.open(listen);

	ACE_DEBUG((LM_INFO, "(%t) Running event loop...\n"));
	ACE_Reactor::instance()->run_reactor_event_loop();

	ACE_DEBUG((LM_INFO, "(%t) server end\n"));
	ACE_RETURN(0);
}
