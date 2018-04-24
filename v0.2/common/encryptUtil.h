#ifndef __ENCRYPT_UTIL_H__
#define __ENCRYPT_UTIL_H__

#include <vector>

#ifdef USE_SOFTHSM
#include <CHsmProxy.h>
void encrypt(CHsmProxy &hsm,CHsmProxy::MechanismType mType, unsigned long hKey,
	     unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vEncryptedData, unsigned long &ulEncryptedDataLen);
void decrypt(CHsmProxy &hsm,CHsmProxy::MechanismType mType, unsigned long hKey, 
	     unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vDecryptedData, unsigned long &ulDecryptedDataLen);
#elif defined(USE_OPTEE)
#include <okey.h>
TEEC_Result encrypt(okey *pO,OperationHandle op,
		    unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vEncryptedData, unsigned long &ulEncryptedDataLen);
TEEC_Result decrypt(okey *pO,OperationHandle op,
		    unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vDecryptedData, unsigned long &ulDecryptedDataLen);
#endif

#endif
