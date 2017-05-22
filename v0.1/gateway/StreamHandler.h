#ifndef __STREAMHANDLER_H__
#define __STREAMHANDLER_H__

#include <ace/Svc_Handler.h>
#include <ace/SOCK_Stream.h>
#include <ace/Reactor_Notification_Strategy.h>
#include "protocol.h"

typedef unsigned int CID;	//Connection ID

class StreamHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> {
private:
	typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;

	ACE_INET_Addr remote_addr_;
	ACE_Reactor_Notification_Strategy noti_;
	static CID sCounter_;
	CID id_;
	unsigned char * serialNo_[SERIAL_NO_SIZE];

	int onSerialNo(const char *buf, size_t dataSize);

public:
	StreamHandler();
	CID id();

	virtual int open(void * = 0);
	virtual int handle_input(ACE_HANDLE handle = ACE_INVALID_HANDLE);
	virtual int handle_output(ACE_HANDLE handle = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask);
};

#endif