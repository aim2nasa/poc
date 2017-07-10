#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <ace/INET_Addr.h>
#include <ace/SSL/SSL_SOCK_Connector.h>
#include <iostream>

#define TARGET_PORT 9088
#define SIZE_BUF	256

static char* SERVER_HOST = "127.0.0.1";
static u_short SERVER_PORT = TARGET_PORT;

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	const char *server_host = SERVER_HOST;
	u_short server_port = SERVER_PORT;
	ACE_INET_Addr remote_addr(server_port, server_host);
	ACE_SSL_SOCK_Connector connector;
	ACE_SSL_SOCK_Stream cli_stream;

	if (connector.connect(cli_stream,remote_addr) == -1)
		ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("(%P|%t) %p\n"),ACE_TEXT("connection failed")),-1);
	else
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("(%P|%t) connected to %C at port %d\n"),remote_addr.get_host_name(),remote_addr.get_port_number()));

	ACE_SSL_Context *context = ACE_SSL_Context::instance();
	if(!context->check_host(remote_addr, cli_stream.ssl())) 
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("check_host failed\n")), -1);

	int nRtn = 0;
	char buffer[SIZE_BUF];
	std::cout << "press q and enter to finish" << std::endl;
	while (true){
		std::cout << ": ";
		fgets(buffer, sizeof(buffer), stdin);

		if (ACE_OS::strcmp(buffer, "q\n") == 0)
			break;

		if ((nRtn = cli_stream.send_n(buffer, SIZE_BUF)) == -1) {
			ACE_DEBUG((LM_DEBUG, "(%P|%t) Error send_n(%d)\n", nRtn));
			break;
		}

		ACE_DEBUG((LM_DEBUG, "(%P|%t) %dbytes sent\n", nRtn));

		// recv
		char recv_buff[SIZE_BUF] = { 0 };
		if ((nRtn = cli_stream.recv_n(recv_buff, sizeof(recv_buff))) == -1) {
			ACE_ERROR((LM_ERROR, "(%P|%t) Error recv_n(%d)\n", nRtn));
			break;
		}
		else
			ACE_DEBUG((LM_DEBUG, "(%P|%t) %dbytes received:%s\n", nRtn, recv_buff));
	}

	if (cli_stream.close() == -1)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "close"), -1);

	ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) SSL client end\n")));
	ACE_RETURN(0);
}