#include <iostream>
#include "ace/SOCK_Connector.h" 
#include "ace/INET_Addr.h" 
#include "ace/Log_Msg.h" 
#include "ace/OS_NS_stdio.h" 
#include "ace/OS_NS_string.h" 
#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_stdlib.h"

#define SIZE_BUF 256

static char* SERVER_HOST = "127.0.0.1";
static u_short SERVER_PORT = 9870;

int main(int argc, char *argv[])
{
	const char *server_host = argc > 1 ? argv[1] : SERVER_HOST;
	u_short server_port = argc > 2 ? ACE_OS::atoi(argv[2]) : SERVER_PORT;
	ACE_DEBUG((LM_INFO, "(%P|%t) server info(addr:%s,port:%d)\n", server_host, server_port));

	ACE_SOCK_Stream client_stream;
	ACE_INET_Addr remote_addr(server_port, server_host);
	ACE_SOCK_Connector connector;

	ACE_DEBUG((LM_DEBUG, "(%P|%t) Starting connect to %s: %d \n", remote_addr.get_host_name(), remote_addr.get_port_number()));
	if (connector.connect(client_stream, remote_addr) == -1)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "connection failed"), -1);
	else
		ACE_DEBUG((LM_DEBUG, "(%P|%t) connected to %s \n", remote_addr.get_host_name()));

	size_t nRtn = 0;
	char buffer[SIZE_BUF];
	std::cout << "press q and enter to finish" << std::endl;
	while (true){
		std::cout << ": ";
		ACE_OS::fgets(buffer, sizeof(buffer), stdin);
		buffer[ACE_OS::strlen(buffer)] = 0;

		if (ACE_OS::strcmp(buffer, "q\n") == 0)
			break;

		if ((nRtn = client_stream.send_n(buffer, ACE_OS::strlen(buffer))) == -1) {
			ACE_DEBUG((LM_DEBUG, "(%P|%t) Error send_n(%d)\n", nRtn));
			break;
		}

		ACE_DEBUG((LM_DEBUG, "(%P|%t) %d bytes sent\n", nRtn));

		// recv
		char recv_buff[SIZE_BUF] = { 0 };
		if ((nRtn = client_stream.recv(recv_buff, sizeof(recv_buff))) == -1) {
			ACE_ERROR((LM_ERROR, "(%P|%t) Error recv_n(%d)\n", nRtn));
			break;
		}
		else
			ACE_DEBUG((LM_DEBUG, "(%P|%t) %dbytes received\n", nRtn));

		ACE_DEBUG((LM_DEBUG, "%s\n", recv_buff));
	}

	if (client_stream.close() == -1)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "close"), -1);

	return 0;
}