#include <iostream>
#include "ace/SOCK_Connector.h" 
#include "ace/INET_Addr.h" 
#include "ace/Log_Msg.h" 
#include "ace/OS_NS_stdio.h" 
#include "ace/OS_NS_string.h" 
#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_stdlib.h"
#include "CHsmProxy.h"
#include "testConf.h"
#include "common.h"

#define SIZE_BUF 256

static char* SERVER_HOST = "127.0.0.1";
static u_short SERVER_PORT = 9870;

CHsmProxy hsm;	//��������

void encrypt(CHsmProxy::MechanismType mType, unsigned long hKey, unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vEncryptedData, unsigned long &ulEncryptedDataLen);
void decrypt(CHsmProxy::MechanismType mType, unsigned long hKey, unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vDecryptedData, unsigned long &ulDecryptedDataLen);

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

	hsm.setenv("SOFTHSM2_CONF", ".\\se2Token.conf", 1);
	ACE_ASSERT(hsm.init(SO_PIN, USER_PIN) == 0);

	unsigned long hTagKey, hSeKey;
	hTagKey = hSeKey = 0;	//CK_INVALID_HANDLE = 0

	ACE_ASSERT(hsm.findKey(TAG_KEY_LABEL, sizeof(TAG_KEY_LABEL)-1, hTagKey) == 0);
	ACE_ASSERT(hTagKey != 0);
	ACE_DEBUG((LM_INFO, "(%t) Tag key(%d) retrieved\n", hTagKey));

	ACE_ASSERT(hsm.findKey(SE_KEY_LABEL, sizeof(SE_KEY_LABEL)-1, hSeKey) == 0);
	ACE_ASSERT(hSeKey != 0);
	ACE_DEBUG((LM_INFO, "(%t) SE key(%d) retrieved\n", hSeKey));

	size_t nRtn = 0;
	const int blockSize(0x10);
	const int NumBlock(10);
	const int bufferSize = blockSize*NumBlock;
	char *buffer = new char[bufferSize+1];	//+1:NULL�� �����ϱ� ���ؼ�
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
	ACE_ASSERT(hsm.encryptInit(mType, hKey) == 0);
	ACE_ASSERT(hsm.encrypt(data, dataLen, NULL, &ulEncryptedDataLen) == 0);
	ACE_ASSERT(ulEncryptedDataLen == dataLen);

	vEncryptedData.resize(ulEncryptedDataLen);
	ACE_ASSERT(hsm.encrypt(data,dataLen,&vEncryptedData.front(), &ulEncryptedDataLen) == 0);
	ACE_ASSERT(ulEncryptedDataLen == dataLen);
}

void decrypt(CHsmProxy::MechanismType mType, unsigned long hKey, unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vDecryptedData, unsigned long &ulDecryptedDataLen)
{
	ACE_ASSERT(hsm.decryptInit(mType, hKey) == 0);
	ACE_ASSERT(hsm.decrypt(data, dataLen, NULL, &ulDecryptedDataLen) == 0);

	vDecryptedData.resize(ulDecryptedDataLen);
	ACE_ASSERT(hsm.decrypt(data, dataLen, &vDecryptedData.front(), &ulDecryptedDataLen) == 0);
}