#include <iostream>
#include "ace/SOCK_Connector.h" 
#include "ace/INET_Addr.h" 
#include "ace/Log_Msg.h" 
#include "ace/OS_NS_stdio.h" 
#include "ace/OS_NS_string.h" 
#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_stdlib.h"
#include "library.h"
#include "CToken.h"

#define SIZE_BUF 256
#define SERIAL_NO_SIZE	32

static char* SERVER_HOST = "127.0.0.1";
static u_short SERVER_PORT = 9876;

int prepareSession(CToken &token, const char *label, const char *soPin, const char *userPin);
int createSerialNo(CToken &token, unsigned char *sn, unsigned int snSize);

int main(int argc, char *argv[])
{
	if (argc<6) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("usage:se <host> <post> <label> <soPin> <userPin>\n")));
		ACE_ERROR((LM_ERROR, ACE_TEXT("      host:set 0 for defalut host(localhost)\n")));
		ACE_ERROR((LM_ERROR, ACE_TEXT("      port:set 0 for defalut port(9876)\n")));
		ACE_RETURN(-1);
	}

	const char *server_host;
	ACE_OS::atoi(argv[1]) == 0 ? server_host = SERVER_HOST : server_host = argv[1];

	u_short server_port;
	ACE_OS::atoi(argv[2]) == 0 ? server_port = (u_short)SERVER_PORT : server_port = ACE_OS::atoi(argv[2]);
	ACE_DEBUG((LM_INFO, "(%P|%t) server info(addr:%s,port:%d)\n", server_host, server_port));

	CToken token;
	if (prepareSession(token, argv[3], argv[4], argv[5]) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("prepareSession failed\n")));
		ACE_RETURN(-1);
	}
	ACE_DEBUG((LM_INFO, "(%t) SlotID:%u Token:%s session ready\n", token.slotID(), token.label().c_str()));

	//SE자신의 가상의 시리얼 넘버를 생성
	unsigned char serialNo[SERIAL_NO_SIZE];
	if (createSerialNo(token,serialNo, SERIAL_NO_SIZE) != 0)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "SE serialNo creation failed"), -1);

	ACE_SOCK_Stream client_stream;
	ACE_INET_Addr remote_addr(server_port, server_host);
	ACE_SOCK_Connector connector;

	ACE_DEBUG((LM_DEBUG, "(%P|%t) Starting connect to %s: %d \n", remote_addr.get_host_name(), remote_addr.get_port_number()));
	if (connector.connect(client_stream, remote_addr) == -1)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "connection failed"), -1);
	else
		ACE_DEBUG((LM_DEBUG, "(%P|%t) connected to %s \n", remote_addr.get_host_name()));

	int nRtn = 0;
	char buffer[SIZE_BUF];
	std::cout << "press q and enter to finish" << std::endl;
	while (true){
		std::cout << ": ";
		fgets(buffer, sizeof(buffer), stdin);

		if (ACE_OS::strcmp(buffer, "q\n") == 0)
			break;

		if ((nRtn = client_stream.send_n(buffer, SIZE_BUF)) == -1) {
			ACE_DEBUG((LM_DEBUG, "(%P|%t) Error send_n(%d)\n", nRtn));
			break;
		}

		ACE_DEBUG((LM_DEBUG, "(%P|%t) %dbytes sent\n", nRtn));

		// recv
		char recv_buff[SIZE_BUF] = { 0 };
		if ((nRtn = client_stream.recv_n(recv_buff, sizeof(recv_buff))) == -1) {
			ACE_ERROR((LM_ERROR, "(%P|%t) Error recv_n(%d)\n", nRtn));
			break;
		}
		else
			ACE_DEBUG((LM_DEBUG, "(%P|%t) %dbytes received:%s\n", nRtn, recv_buff));
	}

	if (client_stream.close() == -1)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "close"), -1);

	return 0;
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

	if (token.login(CKU_SO, soPin, (CK_ULONG)strlen(soPin)) != 0) {
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

	if (token.login(CKU_USER, userPin, (CK_ULONG)strlen(userPin)) != 0) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("%s\n"), token._message));
		ACE_RETURN(-1);
	}

	ACE_RETURN(0);
}

int createSerialNo(CToken &token, unsigned char *sn, unsigned int snSize)
{
	if (token.genRandom(sn, snSize) != 0)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "SE serialNo creation failed"), -1);

	ACE_DEBUG((LM_INFO, "(%t) serialNo:"));
	for (unsigned long i = 0; i < snSize; i++) ACE_DEBUG((LM_INFO, "%0x ", serialNo[i]));
	ACE_DEBUG((LM_INFO, "\n"));
}