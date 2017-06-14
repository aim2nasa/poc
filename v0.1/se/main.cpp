#include <iostream>
#include "ace/SOCK_Connector.h" 
#include "ace/INET_Addr.h" 
#include "ace/Log_Msg.h" 
#include "ace/OS_NS_stdio.h" 
#include "ace/OS_NS_string.h" 
#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/Truncate.h"
#include "library.h"
#include "CToken.h"
#include "protocol.h"
#include "CHsmProxy.h"

#define SIZE_BUF (1024*8)

static char* SERVER_HOST = "127.0.0.1";
static u_short SERVER_PORT = 9876;

int createSerialNo(CToken &token, char *sn, unsigned int snSize);
size_t send(ACE_SOCK_Stream &sock, const char *buffer, size_t len);
int sendSerialNo(ACE_SOCK_Stream &sock, const char *serialNo);
int registerKeys(ACE_SOCK_Stream &sock, CK_SESSION_HANDLE hSession);
int onNtfTags(const char *buffer, unsigned int len, CK_SESSION_HANDLE hSession);
int onTagKey(const char *buffer, unsigned int len, CK_SESSION_HANDLE hSession);
int onSeKey(const char *buffer, unsigned int len, CK_SESSION_HANDLE hSession);
int aesKeyInjection(CK_BYTE_PTR key, CK_ULONG keySize, CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE &hKey, const char *label);

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

	CHsmProxy hsm;
	hsm.setenv("SOFTHSM2_CONF", ".\\softhsm2.conf", 1);

	int nRtn;
	if ((nRtn = hsm.init(argv[3], argv[4], argv[5])) != 0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("HSM init failure(%s,%s,%s):%d\n"), argv[3], argv[4], argv[5], nRtn), -1);

	ACE_DEBUG((LM_INFO, "(%t) SlotID:%u Token:%s session ready\n", hsm.slotID(), hsm.token().label().c_str()));

	//SE자신의 가상의 시리얼 넘버를 생성
	char serialNo[SERIAL_NO_SIZE];
	if (createSerialNo(hsm.token(),serialNo, SERIAL_NO_SIZE) != 0)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "SE serialNo creation failed"), -1);

	ACE_DEBUG((LM_INFO, "(%t) serial number created\n"));

	ACE_SOCK_Stream client_stream;
	ACE_INET_Addr remote_addr(server_port, server_host);
	ACE_SOCK_Connector connector;

	ACE_DEBUG((LM_DEBUG, "(%P|%t) Starting connect to %s: %d \n", remote_addr.get_host_name(), remote_addr.get_port_number()));
	if (connector.connect(client_stream, remote_addr) == -1)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "connection failed"), -1);
	else
		ACE_DEBUG((LM_DEBUG, "(%P|%t) connected to %s \n", remote_addr.get_host_name()));

	if (sendSerialNo(client_stream, serialNo) != 0) ACE_RETURN(-1);
	ACE_DEBUG((LM_INFO, "(%t) serial number sent to gateway\n"));

	if (registerKeys(client_stream, hsm.token().session()) != 0) ACE_RETURN(-1);
	ACE_DEBUG((LM_INFO, "(%t) SE key registered\n"));

	if (client_stream.close() == -1)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "close"), -1);

	ACE_DEBUG((LM_INFO, "(%t) SE successfully initialized\n"));
	return 0;
}

int createSerialNo(CToken &token, char *sn, unsigned int snSize)
{
	if (token.genRandom(sn, snSize) != 0)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "SE serialNo creation failed"), -1);

	ACE_DEBUG((LM_INFO, "(%t) serialNo:"));
	for (unsigned long i = 0; i < snSize; i++) ACE_DEBUG((LM_INFO, ACE_TEXT("%0x "), static_cast<unsigned char>(sn[i])));
	ACE_DEBUG((LM_INFO, "\n"));

	ACE_RETURN(0);
}

size_t send(ACE_SOCK_Stream &sock, const char *buffer,size_t len)
{
	return sock.send_n(buffer,len);
}

int sendSerialNo(ACE_SOCK_Stream &sock, const char *serialNo)
{
	size_t send_cnt = 0;

	//프로토콜 포맷 정의
	//{prefix, 8바이트} {dataSize,바이트} {data}

	//prefix
	if ((send_cnt = send(sock, PRF_SERIALNO, PREFIX_SIZE)) == -1)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "Error send(%d), prefix", send_cnt), -1);

	//dataSize
	ACE_INT32 dataSize = ACE_HTONL(ACE_Utils::truncate_cast<ACE_INT32> (SERIAL_NO_SIZE));
	if ((send_cnt = send(sock, reinterpret_cast<const char*>(&dataSize), sizeof(ACE_INT32))) == -1)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "Error send(%d), dataSize", send_cnt), -1);

	//serialNo
	if ((send_cnt = send(sock, reinterpret_cast<const char*>(serialNo), SERIAL_NO_SIZE)) == -1)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "Error send(%d), serialNo", send_cnt), -1);

	ACE_RETURN(0);
}

int registerKeys(ACE_SOCK_Stream &sock, CK_SESSION_HANDLE hSession)
{
	size_t recv_cnt = 0;
	char buffer[SIZE_BUF];

	//prefix
	if ((recv_cnt = sock.recv_n(buffer, PREFIX_SIZE)) == -1)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "Error recv_n(%d), prefix", recv_cnt), -1);

	buffer[PREFIX_SIZE] = 0;
	std::string prefix = buffer;

	//dataSize
	ACE_INT32 dataSize;
	if ((recv_cnt = sock.recv_n(&dataSize, sizeof(ACE_INT32))) <= 0)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "Error recv_n(%d), dataSize", recv_cnt), -1);

	//data
	if (dataSize > 0) {
		if ((recv_cnt = sock.recv_n(buffer, dataSize)) <= 0)
			ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "Error recv_n(%d), data", recv_cnt), -1);
	}

	if (prefix == PRF_NTF_TAGS) onNtfTags(buffer, dataSize, hSession);
	ACE_RETURN(0);
}

int onNtfTags(const char *buffer, unsigned int len, CK_SESSION_HANDLE hSession)
{
	ACE_ASSERT(len%AES_KEY_SIZE == 0);

	unsigned int count = len / AES_KEY_SIZE;
	for (unsigned int i = 0; i < count; i++) {
		(i == 0) ? onTagKey(buffer,AES_KEY_SIZE,hSession) : onSeKey(buffer,AES_KEY_SIZE,hSession);
		buffer += AES_KEY_SIZE;
	}
	ACE_RETURN(0);
}

int onTagKey(const char *buffer, unsigned int len, CK_SESSION_HANDLE hSession)
{
	ACE_ASSERT(len == AES_KEY_SIZE);

	CK_OBJECT_HANDLE hTagKey = CK_INVALID_HANDLE;
	if (aesKeyInjection((CK_BYTE_PTR)(buffer) ,len, hSession, hTagKey, TAG_KEY_LABEL)!=0) 
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%t) Error in aesKeyInjection(session:%d)\n"), hSession), -1);

	ACE_ASSERT(hTagKey != CK_INVALID_HANDLE);
	ACE_RETURN(0);
}

int onSeKey(const char *buffer, unsigned int len, CK_SESSION_HANDLE hSession)
{
	ACE_ASSERT(len == AES_KEY_SIZE);

	CK_OBJECT_HANDLE hSeKey = CK_INVALID_HANDLE;
	if (aesKeyInjection((CK_BYTE_PTR)(buffer), len, hSession, hSeKey, SE_KEY_LABEL) != 0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%t) Error in aesKeyInjection(session:%d)\n"), hSession), -1);

	ACE_ASSERT(hSeKey != CK_INVALID_HANDLE);
	ACE_RETURN(0);
}

int aesKeyInjection(CK_BYTE_PTR key, CK_ULONG keySize, CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE &hKey, const char *label)
{
	//원하는 키값으로 객체를 생성한다.
	CK_MECHANISM mechanism = { CKM_AES_KEY_GEN, NULL_PTR, 0 };
	CK_BBOOL bTrue = CK_TRUE;
	CK_BBOOL bFalse = CK_FALSE;
	CK_OBJECT_CLASS secretClass = CKO_SECRET_KEY;
	CK_KEY_TYPE keyType = CKK_GENERIC_SECRET;
	CK_ATTRIBUTE attribs[] = {
		{ CKA_VALUE, key, keySize },
		{ CKA_EXTRACTABLE, &bFalse, sizeof(bFalse) },
		{ CKA_CLASS, &secretClass, sizeof(secretClass) },
		{ CKA_KEY_TYPE, &keyType, sizeof(keyType) },
		{ CKA_TOKEN, &bTrue, sizeof(bTrue) },
		{ CKA_PRIVATE, &bFalse, sizeof(bFalse) },
		{ CKA_SENSITIVE, &bFalse, sizeof(bTrue) },
		{ CKA_LABEL, (CK_UTF8CHAR_PTR)label, strlen(label) }
	};

	CK_RV rv;
	hKey = CK_INVALID_HANDLE;
	if((rv=C_CreateObject(hSession, attribs, sizeof(attribs) / sizeof(CK_ATTRIBUTE), &hKey))!=CKR_OK)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%t) Error(0x%x) in C_CreateObject(session:%d)\n"), rv,hSession), -1);

	ACE_RETURN(0);
}