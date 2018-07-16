#ifndef __OKEY__
#define __OKEY__

#include <tee_client_api.h>

#define TEE_STORAGE_PRIVATE			0x00000001
#define TEE_STORAGE_PRIVATE_REE			0x80000000
#define TEE_STORAGE_PRIVATE_RPMB		0x80000100
#define TEE_STORAGE_PRIVATE_SQL_RESERVED	0x80000200

#define TEE_AES_BLOCK_SIZE			16UL
#define TEE_DATA_FLAG_ACCESS_READ		0x00000001
#define TEE_DATA_FLAG_ACCESS_WRITE		0x00000002
#define TEE_DATA_FLAG_ACCESS_WRITE_META		0x00000004
#define TEE_DATA_FLAG_SHARE_READ		0x00000010
#define TEE_DATA_FLAG_SHARE_WRITE		0x00000020
#define TEE_DATA_FLAG_OVERWRITE			0x00000400

typedef void* OperationHandle;

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct _optee_key_context {
		TEEC_Context *ctx;
		TEEC_Session *session;
		uint32_t error;
		size_t shMemSize;
	} okey;

	typedef struct _enum_object {
		uint8_t *id;
		uint32_t idSize;
	} eObj;

	typedef struct _enum_object_list {
		eObj *object;
		struct _enum_object_list *next;
	} eObjList;

	typedef enum {
		PRIVATE = TEE_STORAGE_PRIVATE,
		REE = TEE_STORAGE_PRIVATE_REE,
		RPMB = TEE_STORAGE_PRIVATE_RPMB,
		SQLRESERVED = TEE_STORAGE_PRIVATE_SQL_RESERVED
	} storageId;

	TEEC_Result initializeContext(const char *name,okey *o);
	TEEC_Result openSession(okey *o,uint32_t connectionMethod,
				const void *connectionData,TEEC_Operation *operation);
	TEEC_Result keyGen(okey *o,storageId sid,const char *keyFileName,uint32_t flags,uint32_t keySize);
	TEEC_Result keyOpen(okey *o,storageId sid,const char *keyFileName,uint32_t flags,uint32_t *keyObj);
	TEEC_Result keyInject(okey *o,storageId sid,const char *keyFileName,uint8_t *keyBuffer,size_t keySize,uint32_t flags);
	TEEC_Result keyGetObjectBufferAttribute(okey *o,uint32_t keyObj,uint32_t attrId,void *buffer,size_t *bufferSize);
	TEEC_Result keyEnumObjectList(okey *o,storageId sid,eObjList **list);
	int keyFreeEnumObjectList(eObjList *list);
	TEEC_Result keyAllocOper(okey *o,bool bEnc,uint32_t keyObj,OperationHandle *encOp);
	TEEC_Result keyFreeOper(okey *o,OperationHandle encOp);
	TEEC_Result keySetkeyOper(okey *o,OperationHandle encOp,uint32_t keyObj);
	TEEC_Result keyClose(okey *o,uint32_t keyObj);
	TEEC_Result keyUnlink(okey *o,uint32_t keyObj);
	TEEC_Result allocShm(okey *o,TEEC_SharedMemory *shm,size_t size);
	void freeShm(TEEC_SharedMemory *shm);
	TEEC_Result cipherInit(okey *o,OperationHandle encOp,uint8_t shMemFactor);
	TEEC_Result cipherUpdate(okey *o,OperationHandle encOp,uint8_t *inBuf,size_t inBufSize);
	TEEC_Result cipherDoFinal(okey *o,OperationHandle encOp,uint8_t *inBuf,size_t inBufSize);
	void cipherClose();
	TEEC_SharedMemory *outSharedMemory();
	void closeSession(okey *o);
	void finalizeContext(okey *o);

#ifdef __cplusplus
}
#endif

#endif
