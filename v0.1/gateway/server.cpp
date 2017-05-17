#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <ace/INET_Addr.h>
#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>
#include "StreamHandler.h"
#include <iostream>
#include "library.h"

#define SERVER_PORT 98765

using namespace std;

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	u_short server_port = argc > 1 ? ACE_OS::atoi(argv[1]) : SERVER_PORT;
	ACE_DEBUG((LM_INFO, "(%t) gateway start at port:%d\n", server_port));

	void* module;
	CK_FUNCTION_LIST_PTR p11;
	if (loadLibOnly(&module, &p11) == -1) {
		cout << "ERROR: loadLib" << endl;
		ACE_RETURN(-1);
	}
	cout << "HSM library loaded" << endl;

	CK_RV rv;
	if ((rv = p11->C_Initialize(NULL_PTR)) != CKR_OK) {
		cout << "ERROR: C_Initialize" << endl;
		ACE_RETURN(-1);
	}
	cout << "HSM library initialized" << endl;

	ACE_INET_Addr listen;
	listen.set((u_short)SERVER_PORT);
	ACE_Acceptor<StreamHandler, ACE_SOCK_ACCEPTOR> acceptor;
	acceptor.open(listen);
	ACE_Reactor::instance()->run_reactor_event_loop();

	unloadLib(module);
	ACE_DEBUG((LM_INFO, "(%t) gateway end\n"));
	ACE_RETURN(0);
}