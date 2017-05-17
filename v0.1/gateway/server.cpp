#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <ace/INET_Addr.h>
#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>
#include "StreamHandler.h"
#include "CToken.h"
#include "Helper.h"

#define SERVER_PORT 9876

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
	ACE_DEBUG((LM_INFO, "(%t) gateway start at port:%u\n", server_port));

	CToken token;
	if (prepareSession(token, argv[2], argv[3], argv[4]) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("prepareSession failed\n")));
		ACE_RETURN(-1);
	}
	ACE_DEBUG((LM_INFO, "(%t) SlotID:%u Token:%s session ready\n", token.slotID(), token.label().c_str()));

	//GwKey(AES키를 생성)
	CK_ULONG keySize = 32;
	CK_OBJECT_HANDLE hGw = CK_INVALID_HANDLE;
	if (gatewayKey(token, keySize, hGw) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("gatewayKey creation failed\n")));
		ACE_RETURN(-1);
	}
	ACE_DEBUG((LM_INFO, "(%t) AES key for gateway created\n"));

	ACE_INET_Addr listen;
	listen.set((u_short)SERVER_PORT);
	ACE_Acceptor<StreamHandler, ACE_SOCK_ACCEPTOR> acceptor;
	acceptor.open(listen);
	ACE_Reactor::instance()->run_reactor_event_loop();

	ACE_DEBUG((LM_INFO, "(%t) gateway end\n"));
	ACE_RETURN(0);
}