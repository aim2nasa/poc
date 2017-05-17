#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <ace/INET_Addr.h>
#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>
#include "StreamHandler.h"
#include "CToken.h"

#define SERVER_PORT 98765

using namespace std;

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	u_short server_port = argc > 1 ? ACE_OS::atoi(argv[1]) : SERVER_PORT;
	ACE_DEBUG((LM_INFO, "(%t) gateway start at port:%d\n", server_port));

	CToken token;
	if (token.initialize() != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}
	ACE_DEBUG((LM_INFO, "(%t) HSM library initialized\n"));

	ACE_INET_Addr listen;
	listen.set((u_short)SERVER_PORT);
	ACE_Acceptor<StreamHandler, ACE_SOCK_ACCEPTOR> acceptor;
	acceptor.open(listen);
	ACE_Reactor::instance()->run_reactor_event_loop();

	ACE_DEBUG((LM_INFO, "(%t) gateway end\n"));
	ACE_RETURN(0);
}