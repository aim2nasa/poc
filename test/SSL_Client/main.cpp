#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <ace/INET_Addr.h>
#include <ace/SSL/SSL_SOCK_Connector.h>

#define TARGET_PORT 9088

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

	ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) SSL client end\n")));
	ACE_RETURN(0);
}