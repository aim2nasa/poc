#include <ace/OS.h>
#include <ace/Log_Msg.h>

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) SSL server end\n")));
	ACE_RETURN(0);
}