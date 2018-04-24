#include <iostream>
#include "ace/SOCK_Connector.h" 
#include "ace/INET_Addr.h" 
#include "ace/Log_Msg.h" 
#include "ace/OS_NS_stdio.h" 
#include "ace/OS_NS_string.h" 
#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_sys_time.h"

#ifdef USE_SOFTHSM
#include "CHsmProxy.h"
#elif defined(USE_OPTEE)
#include <common.h>
#include <okey.h>
#endif
#include "../common/encryptUtil.h"

#define SIZE_BUF 256

static char* SERVER_HOST = "127.0.0.1";
static u_short SERVER_PORT = 9870;

#ifdef USE_SOFTHSM
CHsmProxy hsm;	//전역변수
int authenticate(ACE_SOCK_Stream &stream, CHsmProxy::MechanismType mType, unsigned long hKey, char* buffer, const int bufferSize);
#elif defined(USE_OPTEE)
okey o;
int authenticate(ACE_SOCK_Stream &stream, OperationHandle encOp, OperationHandle decOp, char* buffer, const int bufferSize);
#endif

int main(int argc, char *argv[])
{
#ifdef USE_SOFTHSM
	ACE_DEBUG((LM_INFO, "configured to use softhsm\n"));
	if (argc<4) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("usage:tokenClient <host> <port> <userPin>\n")));
		ACE_ERROR((LM_ERROR, ACE_TEXT("      host:set 0 for defalut host(localhost)\n")));
		ACE_ERROR((LM_ERROR, ACE_TEXT("      port:set 0 for defalut port(9870)\n")));
		ACE_RETURN(-1);
	}
#elif defined(USE_OPTEE)
	ACE_DEBUG((LM_INFO, "configured to use optee\n"));
	if (argc<3) {
		ACE_ERROR((LM_ERROR, ACE_TEXT("usage:tokenClient <host> <port>\n")));
		ACE_ERROR((LM_ERROR, ACE_TEXT("      host:set 0 for defalut host(localhost)\n")));
		ACE_ERROR((LM_ERROR, ACE_TEXT("      port:set 0 for defalut port(9870)\n")));
		ACE_RETURN(-1);
	}
#else
	ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Error predefine missing, define USE_SOFTHSM or USE_OPTEE\n")), -1);
#endif

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

	const int blockSize(0x10);
	const int NumBlock(10);
	const int bufferSize = blockSize*NumBlock;

#ifdef USE_SOFTHSM
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
#elif defined(USE_OPTEE)
	TEEC_Result res = initializeContext(NULL,&o);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("initializeContext failed 0x%x"), res), -1);

	res = openSession(&o,TEEC_LOGIN_PUBLIC,NULL,NULL);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("openSession failed 0x%x"), res), -1);

	uint32_t flags = TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_SHARE_READ; 
	uint32_t tagKey;
	res = keyOpen(&o,TEE_STORAGE_PRIVATE,TAG_KEY_LABEL,flags,&tagKey);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("keyOpen(%s) failed 0x%x"), TAG_KEY_LABEL,res), -1);
	ACE_DEBUG((LM_INFO, "(%t) Tag key(0x%x) retrieved\n", tagKey));

	size_t keySize = 256;	//TODO hardcode for the time being. need to be configurable and monitored through API in libokey

	OperationHandle encOp;
	res = keyAllocOper(&o,true,keySize,&encOp);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("keyAllocOper(encode) failed 0x%x"), res), -1);
	ACE_DEBUG((LM_INFO, "(%t) encode operation handle:0x%x\n", encOp));

	OperationHandle decOp;
	res = keyAllocOper(&o,false,keySize,&decOp);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("keyAllocOper(decode) failed 0x%x"), res), -1);
	ACE_DEBUG((LM_INFO, "(%t) decode operation handle:0x%x\n", decOp));

	//inject key for operations
	res = keySetkeyOper(&o,encOp,tagKey);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("encode keySetkeyOper(0x%x) failed 0x%x"), tagKey,res), -1);
	ACE_DEBUG((LM_INFO, "(%t) setkey(0x%x) for encode operation(0x%x)\n", tagKey,encOp));

	res = keySetkeyOper(&o,decOp,tagKey);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("decode keySetkeyOper(0x%x) failed 0x%x"), tagKey,res), -1);
	ACE_DEBUG((LM_INFO, "(%t) setkey(0x%x) for decode operation(0x%x)\n", tagKey,decOp));

	uint8_t shMemFactor = NumBlock;
	res = cipherInit(&o,encOp,shMemFactor);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("encode cipherInit(0x%x,%d) failed 0x%x"), encOp,shMemFactor,res), -1);
	ACE_DEBUG((LM_INFO, "(%t) encode cipher initialized(0x%x) with shMemFactor=%d\n", encOp,shMemFactor));

	res = cipherInit(&o,decOp,shMemFactor);
	if(res!=TEEC_SUCCESS)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ") ACE_TEXT("decode cipherInit(0x%x,%d) failed 0x%x"), decOp,shMemFactor,res), -1);
	ACE_DEBUG((LM_INFO, "(%t) decode cipher initialized(0x%x) with shMemFactor=%d\n", decOp,shMemFactor));
#endif

	size_t nRtn = 0;
	char *buffer = new char[bufferSize+1];	//+1:NULL을 삽입하기 위해서

	int nAuth;
#ifdef USE_SOFTHSM
	if ((nAuth = authenticate(client_stream, CHsmProxy::AES_ECB, hTagKey, buffer, bufferSize)) != 0) {
#elif defined(USE_OPTEE)
	if ((nAuth = authenticate(client_stream, encOp, decOp, buffer, bufferSize)) != 0) {
#endif
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
#ifdef USE_SOFTHSM
		encrypt(hsm,CHsmProxy::AES_ECB, hTagKey, (unsigned char*)buffer, bufferSize, vEncryptedData, ulEncryptedDataLen);
#elif defined(USE_OPTEE)
		TEEC_Result res = encrypt(&o,encOp,(unsigned char*)buffer, bufferSize, vEncryptedData, ulEncryptedDataLen);
		if(res!=TEEC_SUCCESS){
			ACE_DEBUG((LM_ERROR, "(%P|%t) encrypt error res=0x%x\n", res));
			break;
		}
#endif

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
#ifdef USE_SOFTHSM
		decrypt(hsm,CHsmProxy::AES_ECB, hTagKey, (unsigned char*)buffer, (unsigned long)nRtn, vDecryptedData, ulDecryptedDataLen);
#elif defined(USE_OPTEE)
		TEEC_Result res = decrypt(&o,decOp,(unsigned char*)buffer, (unsigned long)nRtn, vDecryptedData, ulDecryptedDataLen);
		if(res!=TEEC_SUCCESS){
			ACE_DEBUG((LM_ERROR, "(%P|%t) decrypt error res=0x%x\n", res));
			break;
		}
#endif
		ACE_DEBUG((LM_INFO, "Decrypt stream:%s\n", &vDecryptedData.front()));
	}
	delete[] buffer;

	if (client_stream.close() == -1)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "close"), -1);

	return 0;
}

#ifdef USE_SOFTHSM
int authenticate(ACE_SOCK_Stream &stream, CHsmProxy::MechanismType mType, unsigned long hKey, char* buffer, const int bufferSize)
#elif defined(USE_OPTEE)
int authenticate(ACE_SOCK_Stream &stream, OperationHandle encOp, OperationHandle decOp, char* buffer, const int bufferSize)
#endif
{
	ACE_OS::memset(buffer, 0, bufferSize);
	ACE_OS::memcpy(buffer, "AuthRequest", sizeof("AuthRequest") - 1);	//인증요청 메세지

	unsigned long ulEncryptedDataLen;
	std::vector<unsigned char> vEncryptedData;
#ifdef USE_SOFTHSM
	encrypt(hsm,CHsmProxy::AES_ECB, hKey, (unsigned char*)buffer, bufferSize, vEncryptedData, ulEncryptedDataLen);
#elif defined(USE_OPTEE)
	TEEC_Result res = encrypt(&o,encOp, (unsigned char*)buffer, bufferSize, vEncryptedData, ulEncryptedDataLen);
	if(res!=TEEC_SUCCESS) 
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("encrypt error res=0x%x\n"),res), -1);
#endif

	ACE_DEBUG((LM_DEBUG, "(%P|%t) ulEncryptedDataLen:%d\n",ulEncryptedDataLen));

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
#ifdef USE_SOFTHSM
	decrypt(hsm,mType, hKey, (unsigned char*)buffer, (unsigned long)size, vDecryptedData, ulDecryptedDataLen);
#elif defined(USE_OPTEE)
	TEEC_Result res = decrypt(&o,decOp, (unsigned char*)buffer, (unsigned long)size, vDecryptedData, ulDecryptedDataLen);
	if(res!=TEEC_SUCCESS) 
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("decrypt error res=0x%x\n"),res), -1);
#endif

	std::string str = (char*)&vDecryptedData.front();
	if (str != "AuthRequest:Done") {
		ACE_DEBUG((LM_INFO, "(%P|%t) AuthRequest failed:%s\n",str.c_str()));
		return 1;
	}

	ACE_DEBUG((LM_INFO, "(%P|%t) AuthRequest:Done\n"));
	return 0;
}
