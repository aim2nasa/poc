#include "encryptUtil.h"
#include <string.h>

#ifdef USE_SOFTHSM
#include <ace/Assert.h>
void encrypt(CHsmProxy &hsm,CHsmProxy::MechanismType mType, unsigned long hKey,
	     unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vEncryptedData, unsigned long &ulEncryptedDataLen)
{
	int nRtn = hsm.encryptInit(mType, hKey);
	ACE_ASSERT( nRtn == 0);
	nRtn = hsm.encrypt(data, dataLen, NULL, &ulEncryptedDataLen);
	ACE_ASSERT( nRtn == 0);
	ACE_ASSERT(ulEncryptedDataLen == dataLen);

	vEncryptedData.resize(ulEncryptedDataLen);
	nRtn = hsm.encrypt(data, dataLen, &vEncryptedData.front(), &ulEncryptedDataLen);
	ACE_ASSERT( nRtn == 0);
	ACE_ASSERT(ulEncryptedDataLen == dataLen);
}

void decrypt(CHsmProxy &hsm,CHsmProxy::MechanismType mType, unsigned long hKey, 
	     unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vDecryptedData, unsigned long &ulDecryptedDataLen)
{
	int nRtn = hsm.decryptInit(mType, hKey);
	ACE_ASSERT( nRtn == 0);
	nRtn = hsm.decrypt(data, dataLen, NULL, &ulDecryptedDataLen);
	ACE_ASSERT( nRtn == 0);

	vDecryptedData.resize(ulDecryptedDataLen);
	nRtn = hsm.decrypt(data, dataLen, &vDecryptedData.front(), &ulDecryptedDataLen);
	ACE_ASSERT( nRtn == 0);
}
#elif defined(USE_OPTEE)
TEEC_Result encrypt(okey *pO,OperationHandle op,
		    unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vEncryptedData, unsigned long &ulEncryptedDataLen)
{
	TEEC_Result res = cipherUpdate(pO,op,data,dataLen);
	if(res!=TEEC_SUCCESS) return res;

	ulEncryptedDataLen = outSharedMemory()->size;
	vEncryptedData.resize(ulEncryptedDataLen);
	memcpy(&vEncryptedData.front(),outSharedMemory()->buffer,outSharedMemory()->size);
	return res;
}

TEEC_Result decrypt(okey *pO,OperationHandle op,
		    unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vDecryptedData, unsigned long &ulDecryptedDataLen)
{
	TEEC_Result res = cipherUpdate(pO,op,data,dataLen);
	if(res!=TEEC_SUCCESS) return res;

	ulDecryptedDataLen = outSharedMemory()->size;
	vDecryptedData.resize(ulDecryptedDataLen);
	memcpy(&vDecryptedData.front(),outSharedMemory()->buffer,outSharedMemory()->size);
	return res;
}
#endif
