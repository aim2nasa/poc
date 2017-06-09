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
#include "CHsmProxy.h"
#include "testConf.h"
#include "common.h"

#include "CClientAcceptor.h"

#define SERVER_PORT 9870

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	u_short server_port = argc > 1 ? ACE_OS::atoi(argv[1]) : SERVER_PORT;
	ACE_DEBUG((LM_INFO, "(%t) server start at port:%d\n", server_port));

	ACE_INET_Addr listen;
	listen.set(SERVER_PORT);

	CHsmProxy hsm;
	hsm.setenv("SOFTHSM2_CONF", ".\\se1Token.conf", 1);
	if (hsm.init(SO_PIN, USER_PIN) != 0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("HSM init failure(soPin:%s,userPin:%s)"), SO_PIN, USER_PIN), -1);

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

	CClientAcceptor acceptor;
	acceptor.pHsm_ = &hsm;
	acceptor.hTagKey_ = hTagKey;
	acceptor.hSeKey_ = hSeKey;
	acceptor.open(listen);

	ACE_DEBUG((LM_INFO, "(%t) Running event loop...\n"));
	ACE_Reactor::instance()->run_reactor_event_loop();

	ACE_DEBUG((LM_INFO, "(%t) server end\n"));
	ACE_RETURN(0);
}