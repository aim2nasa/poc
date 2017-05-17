#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <ace/INET_Addr.h>
#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>
#include "StreamHandler.h"
#include "CToken.h"

int prepareSession(CToken &token, const char *label, const char *soPin, const char *userPin);

#define SERVER_PORT 98765

using namespace std;

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	if (argc<5) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("usage:gateway <port> <label> <soPin> <userPin>\n")));
		ACE_ERROR((LM_ERROR, ACE_TEXT("      port:set 0 for defalut port(98765)\n")));
		ACE_RETURN(-1);
	}

	u_short server_port;
	ACE_OS::atoi(argv[1]) == 0 ? server_port = (u_short)SERVER_PORT : server_port = ACE_OS::atoi(argv[1]);
	ACE_DEBUG((LM_INFO, "(%t) gateway start at port:%d\n", server_port));

	CToken token;
	if (prepareSession(token, argv[2], argv[3], argv[4]) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("prepareSession failed\n")));
		ACE_RETURN(-1);
	}
	ACE_DEBUG((LM_INFO, "(%t) slot:%u token session ready\n",token.slotID()));

	ACE_INET_Addr listen;
	listen.set((u_short)SERVER_PORT);
	ACE_Acceptor<StreamHandler, ACE_SOCK_ACCEPTOR> acceptor;
	acceptor.open(listen);
	ACE_Reactor::instance()->run_reactor_event_loop();

	ACE_DEBUG((LM_INFO, "(%t) gateway end\n"));
	ACE_RETURN(0);
}

int prepareSession(CToken &token, const char *label, const char *soPin, const char *userPin)
{
	if (token.initialize() != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}
	ACE_DEBUG((LM_INFO, "(%t) HSM library initialized\n"));

	CK_ULONG ulSlotCount;
	if (token.slotCount(ulSlotCount) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}
	ACE_DEBUG((LM_INFO, "(%t) number of slots:%d\n", ulSlotCount));

	if (token.initToken(ulSlotCount - 1, soPin, (CK_ULONG)strlen(soPin), label, (CK_ULONG)strlen(label)) != 0) { //slotID: 디폴트로 들어가는 한개의 카운트를 제외한다. 슬롯이 하나도 없을때도 카운트는 1로 나오므로
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}

	if (token.openSession() != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}

	if (token.login(CKU_SO,soPin,(CK_ULONG)strlen(soPin)) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}

	if (token.initPin(userPin, (CK_ULONG)strlen(userPin)) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}

	if (token.logout() != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}

	if (token.login(CKU_USER,userPin,(CK_ULONG)strlen(userPin)) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}

	ACE_RETURN(0);
}