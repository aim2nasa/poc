#include <iostream>
#include "ace/SOCK_Connector.h" 
#include "ace/INET_Addr.h" 
#include "ace/Log_Msg.h" 
#include "ace/OS_NS_stdio.h" 
#include "ace/OS_NS_string.h" 
#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_sys_time.h"
#include "CHsmProxy.h"
#include "common.h"

#define SIZE_BUF 256

static char* SERVER_HOST = "127.0.0.1";
static u_short SERVER_PORT = 9870;

CHsmProxy hsm;	//전역변수

void encrypt(CHsmProxy::MechanismType mType, unsigned long hKey, unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vEncryptedData, unsigned long &ulEncryptedDataLen);
void decrypt(CHsmProxy::MechanismType mType, unsigned long hKey, unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vDecryptedData, unsigned long &ulDecryptedDataLen);
int authenticate(ACE_SOCK_Stream &stream, CHsmProxy::MechanismType mType, unsigned long hKey, char* buffer, const int bufferSize);

int main(int argc, char *argv[])
{
	if (argc<4) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("usage:tokenClient <host> <port> <userPin>\n")));
		ACE_ERROR((LM_ERROR, ACE_TEXT("      host:set 0 for defalut host(localhost)\n")));
		ACE_ERROR((LM_ERROR, ACE_TEXT("      port:set 0 for defalut port(9870)\n")));
		ACE_RETURN(-1);
	}

	char *server_host = NULL;
	ACE_OS::atoi(argv[1]) == 0 ? server_host = SERVER_HOST : server_host = argv[1];
	u_short server_port;
	ACE_OS::atoi(argv[2]) == 0 ? server_port = (u_short)SERVER_PORT : server_port = ACE_OS::atoi(argv[2]);
	ACE_DEBUG((LM_INFO, "(%P|%t) server info(addr:%s,port:%d)\n", server_host, server_port));

	ACE_SOCK_Stream client_stream;
	ACE_INET_Addr remote_addr(server_port, server_host);
	ACE_SOCK_Connector connector;

	ACE_DEBUG((LM_DEBUG, "(%P|%t) Starting connect to %s: %d \n", remote_addr.get_host_name(), remote_addr.get_port_number()));
	if (connector.connect(client_stream, remote_addr) == -1)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "connection failed"), -1);
	else
		ACE_DEBUG((LM_DEBUG, "(%P|%t) connected to %s \n", remote_addr.get_host_name()));

#ifndef _WIN32
	hsm.setenv("SOFTHSM2_CONF", "./softhsm2-linux.conf", 1);
#else
	hsm.setenv("SOFTHSM2_CONF", ".\\softhsm2.conf", 1);
#endif
	int nInit;
	if ((nInit = hsm.init(argv[3])) != 0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("HSM init failure(userPin:%s):%d (%s)"), argv[3], nInit, hsm.message_), -1);

	unsigned long hTagKey, hSeKey;
	hTagKey = hSeKey = 0;	//CK_INVALID_HANDLE = 0

	if (hsm.findKey(TAG_KEY_LABEL, sizeof(TAG_KEY_LABEL)-1, hTagKey) != 0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("Tag key(%s) not found\n"), TAG_KEY_LABEL), -1);

	ACE_DEBUG((LM_INFO, "(%t) SlotID:%u\n", hsm.slotID()));

	ACE_ASSERT(hTagKey != 0);
	ACE_DEBUG((LM_INFO, "(%t) Tag key(%d) retrieved\n", hTagKey));

	if (hsm.findKey(SE_KEY_LABEL, sizeof(SE_KEY_LABEL)-1, hSeKey) != 0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("SE key(%s) not found\n"), SE_KEY_LABEL), -1);

	ACE_ASSERT(hSeKey != 0);
	ACE_DEBUG((LM_INFO, "(%t) SE key(%d) retrieved\n", hSeKey));

	size_t nRtn = 0;
	const int blockSize(0x10);
	const int NumBlock(10);
	const int bufferSize = blockSize*NumBlock;
	char *buffer = new char[bufferSize+1];	//+1:NULL을 삽입하기 위해서

	int nAuth;
	if ((nAuth = authenticate(client_stream, CHsmProxy::AES_ECB, hTagKey, buffer, bufferSize)) != 0) {
		std::cout << "Authentication failure : " <<nAuth<< std::endl;
		return -1;
	}
	std::cout << "Authentication successful" << std::endl;

	std::cout << "press q and enter to finish" << std::endl;
	while (true){
		std::cout << ": ";
		ACE_OS::fgets(buffer, bufferSize, stdin);
		buffer[ACE_OS::strlen(buffer)] = 0;

		if (ACE_OS::strcmp(buffer, "q\n") == 0)
			break;

		unsigned long ulEncryptedDataLen;
		std::vector<unsigned char> vEncryptedData;
		encrypt(CHsmProxy::AES_ECB, hTagKey, (unsigned char*)buffer, bufferSize, vEncryptedData, ulEncryptedDataLen);

		if ((nRtn = client_stream.send_n(&vEncryptedData.front(), ulEncryptedDataLen)) == -1) {
			ACE_DEBUG((LM_DEBUG, "(%P|%t) Error send_n(%d)\n", nRtn));
			break;
		}

		ACE_DEBUG((LM_DEBUG, "(%P|%t) %d bytes sent\n", nRtn));

		// recv
		if ((nRtn = client_stream.recv(buffer, bufferSize)) == -1) {
			ACE_ERROR((LM_ERROR, "(%P|%t) Error recv_n(%d)\n", nRtn));
			break;
		}
		else
			ACE_DEBUG((LM_DEBUG, "(%P|%t) %d bytes received\n", nRtn));

		buffer[nRtn] = 0;
		ACE_DEBUG((LM_INFO, "Encrypted stream:%s\n", buffer));

		unsigned long ulDecryptedDataLen;
		std::vector<unsigned char> vDecryptedData;
		decrypt(CHsmProxy::AES_ECB, hTagKey, (unsigned char*)buffer, (unsigned long)nRtn, vDecryptedData, ulDecryptedDataLen);
		ACE_DEBUG((LM_INFO, "Decrypt stream:%s\n", &vDecryptedData.front()));
	}
	delete[] buffer;

	if (client_stream.close() == -1)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "close"), -1);

	return 0;
}

void encrypt(CHsmProxy::MechanismType mType, unsigned long hKey, unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vEncryptedData, unsigned long &ulEncryptedDataLen)
{
	int nRtn = hsm.encryptInit(mType, hKey);
	ACE_ASSERT(nRtn == 0);
	nRtn = hsm.encrypt(data, dataLen, NULL, &ulEncryptedDataLen);
	ACE_ASSERT(nRtn == 0);
	ACE_ASSERT(ulEncryptedDataLen == dataLen);

	vEncryptedData.resize(ulEncryptedDataLen);
	nRtn = hsm.encrypt(data, dataLen, &vEncryptedData.front(), &ulEncryptedDataLen);
	ACE_ASSERT( nRtn == 0);
	ACE_ASSERT(ulEncryptedDataLen == dataLen);
}

void decrypt(CHsmProxy::MechanismType mType, unsigned long hKey, unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vDecryptedData, unsigned long &ulDecryptedDataLen)
{
	int nRtn = hsm.decryptInit(mType, hKey);
	ACE_ASSERT( nRtn == 0);
	nRtn = hsm.decrypt(data, dataLen, NULL, &ulDecryptedDataLen);
	ACE_ASSERT( nRtn == 0);

	vDecryptedData.resize(ulDecryptedDataLen);
	nRtn = hsm.decrypt(data, dataLen, &vDecryptedData.front(), &ulDecryptedDataLen);
	ACE_ASSERT( nRtn == 0);
}

int authenticate(ACE_SOCK_Stream &stream, CHsmProxy::MechanismType mType, unsigned long hKey, char* buffer, const int bufferSize)
{
	ACE_OS::memset(buffer, 0, bufferSize);
	ACE_OS::memcpy(buffer, "AuthRequest", sizeof("AuthRequest") - 1);	//인증요청 메세지

	unsigned long ulEncryptedDataLen;
	std::vector<unsigned char> vEncryptedData;
	encrypt(CHsmProxy::AES_ECB, hKey, (unsigned char*)buffer, bufferSize, vEncryptedData, ulEncryptedDataLen);

	size_t size;
	if ((size = stream.send_n(&vEncryptedData.front(), ulEncryptedDataLen)) == -1) {
		ACE_DEBUG((LM_ERROR, "(%P|%t) Error send_n(%d)\n", size));
		return -1;
	}
	ACE_DEBUG((LM_DEBUG, "(%P|%t) Authentication Request %d bytes sent\n", size));

	ACE_Time_Value waitTime(3);	//3초 동안 오지 않으면 타임아웃
	if ((size = stream.recv(buffer, bufferSize, &waitTime)) == -1) {
		ACE_ERROR((LM_ERROR, "(%P|%t) Error recv_n(%d)\n", size));
		return -2;
	}
	ACE_DEBUG((LM_DEBUG, "(%P|%t) %d bytes received\n", size));

	if (size == 0)	{//서버에서 연결이 끊어진 상태임
		ACE_DEBUG((LM_INFO, "(%P|%t) Authentication failed\n"));
		return -3;
	}

	unsigned long ulDecryptedDataLen;
	std::vector<unsigned char> vDecryptedData;
	decrypt(mType, hKey, (unsigned char*)buffer, (unsigned long)size, vDecryptedData, ulDecryptedDataLen);

	std::string str = (char*)&vDecryptedData.front();
	if (str != "AuthRequest:Done") {
		ACE_DEBUG((LM_INFO, "(%P|%t) AuthRequest failed:%s\n",str.c_str()));
		return 1;
	}

	ACE_DEBUG((LM_INFO, "(%P|%t) AuthRequest:Done\n"));
	return 0;
}
