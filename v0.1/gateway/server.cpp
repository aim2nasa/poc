#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <ace/INET_Addr.h>
#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>
#include "StreamHandler.h"
#include "CCtrlProxy.h"
#include "CToken.h"
#include "Helper.h"
#include "CSeAcceptor.h"
#include "CGwData.h"
#include "common.h"
#include "CHsmProxy.h"

#define SERVER_PORT 9876
#define CONTRL_PORT 9875

using namespace std;

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	ACE_ASSERT(AES_KEY_SIZE <= AES_MAX_KEY_SIZE);
	if (argc<5) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("usage:gateway <port> <label> <soPin> <userPin>\n")));
		ACE_ERROR((LM_ERROR, ACE_TEXT("      port:set 0 for defalut port(98765)\n")));
		ACE_RETURN(-1);
	}

	u_short server_port;
	ACE_OS::atoi(argv[1]) == 0 ? server_port = (u_short)SERVER_PORT : server_port = ACE_OS::atoi(argv[1]);
	ACE_DEBUG((LM_INFO, "(%t) gateway start at port:%u\n", server_port));

	CHsmProxy hsm;
	hsm.setenv("SOFTHSM2_CONF", ".\\softhsm2.conf", 1);

	int nRtn;
	if ((nRtn=hsm.init(argv[2], argv[3], argv[4])) != 0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("HSM init failure(%s,%s,%s):%d (%s)\n"), argv[2], argv[3], argv[4], nRtn, hsm.message_), -1);

	ACE_DEBUG((LM_INFO, "(%t) SlotID:%u Token:%s session ready\n", hsm.slotID(), hsm.token().label().c_str()));

	//GwKey(AES키를 생성)
	if (gatewayKey(hsm.token(), AES_KEY_SIZE, CGwData::getInstance()->hGw_) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("gatewayKey creation failed\n")));
		ACE_RETURN(-1);
	}
	ACE_DEBUG((LM_INFO, "(%t) AES key for gateway created\n"));
	CGwData::getInstance()->token_ = &hsm.token();
	showKey(hsm.token().session(), CGwData::getInstance()->hGw_, "GW");

	ACE_INET_Addr listen;
	listen.set((u_short)SERVER_PORT);
	CSeAcceptor acceptor;	//CSeAcceptor는 CGwData를 자료구조로 갖는 ACE_Acceptor<StreamHandler, ACE_SOCK_ACCEPTOR>에서 상속받은 클래스이다
							//새로운 접속이 생기면 StreamHandler의 open과정에서 이 자료구조에 접속번호화 StreamHandler의 포인터를 등록한다.
	if(acceptor.open(listen)==-1) 
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%t)\n"),ACE_TEXT("acceptor.open")),-1);

	ACE_DEBUG((LM_INFO, "(%t) Port(%d) for SE opened\n", SERVER_PORT));

	ACE_INET_Addr ctrlListen;
	ctrlListen.set((u_short)CONTRL_PORT);
	ACE_Acceptor<CCtrlProxy, ACE_SOCK_ACCEPTOR> ctrlAcceptor;
	if (ctrlAcceptor.open(ctrlListen) == -1)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%t)\n"), ACE_TEXT("ctrlAcceptor.open")), -1);

	ACE_DEBUG((LM_INFO, "(%t) Port(%d) for gatewayCtrl opened\n", CONTRL_PORT));

	ACE_DEBUG((LM_INFO, "(%t) Running event loop...\n"));
	ACE_Reactor::instance()->run_reactor_event_loop();

	CGwData::delInstance();
	ACE_DEBUG((LM_INFO, "(%t) gateway end\n"));
	ACE_RETURN(0);
}