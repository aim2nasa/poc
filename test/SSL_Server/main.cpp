#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <ace/SSL/SSL_Context.h>

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	ACE_SSL_Context *context = ACE_SSL_Context::instance();

	context->certificate("./rootcert.pem", SSL_FILETYPE_PEM);
	context->private_key("./rootkey.pem", SSL_FILETYPE_PEM);

	// ���� �������� ����Ű�� �´��� Ȯ�� �Ѵ�.
	if(context->verify_private_key()!=0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("�������� ����Ű�� ���� �ʽ��ϴ�.\n")), -1);

	ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) SSL server end\n")));
	ACE_RETURN(0);
}