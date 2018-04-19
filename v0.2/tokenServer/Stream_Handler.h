#ifndef __STREAM_HANDLER_H__
#define __STREAM_HANDLER_H__

#include <ace/SOCK_Acceptor.h>
#include <ace/Acceptor.h>
#include <ace/Reactor_Notification_Strategy.h>
#include "CHsmProxy.h"

class Stream_Handler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> {
private:
	typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;

	ACE_INET_Addr remote_addr_;
	ACE_Reactor_Notification_Strategy noti_;
	bool autheProcess_;
#ifdef USE_SOFTHSM
	CHsmProxy *pHsm_;
	unsigned long hTagKey_, hSeKey_;
#endif

public:
	Stream_Handler();

	virtual int open(void * p = 0);
	virtual int handle_input(ACE_HANDLE handle = ACE_INVALID_HANDLE);
	virtual int handle_output(ACE_HANDLE handle = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask);

	int sendAuthRequestResult(unsigned char *data, unsigned long dataLen);

#ifdef USE_SOFTHSM
	void encrypt(CHsmProxy::MechanismType mType, unsigned long hKey, unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vEncryptedData, unsigned long &ulEncryptedDataLen);
	void decrypt(CHsmProxy::MechanismType mType, unsigned long hKey, unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vDecryptedData, unsigned long &ulDecryptedDataLen);
#endif
};

#endif
