#ifndef __STREAMHANDLER_H__
#define __STREAMHANDLER_H__

#include <common.h>
#include <ace/Svc_Handler.h>
#ifdef USE_SSL
#include <ace/SSL/SSL_SOCK_Acceptor.h>
#else
#include <ace/SOCK_Stream.h>
#endif
#include <ace/Reactor_Notification_Strategy.h>
#include "protocol.h"

typedef unsigned int CID;	//Connection ID

class CSeAcceptor;

#ifdef USE_SSL
class StreamHandler : public ACE_Svc_Handler<ACE_SSL_SOCK_Stream, ACE_NULL_SYNCH> {
#else
class StreamHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> {
#endif
private:
#ifdef USE_SSL
	typedef ACE_Svc_Handler<ACE_SSL_SOCK_Stream, ACE_NULL_SYNCH> super;
#else
	typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;
#endif

	ACE_INET_Addr remote_addr_;
	ACE_Reactor_Notification_Strategy noti_;
	static CID sCounter_;
	CID id_;
	char serialNo_[SERIAL_NO_SIZE];
	CSeAcceptor	*seAcceptor_;

	int onSerialNo(const char *buf, size_t dataSize);

public:
	StreamHandler();
	CID id();
	char* serialNo();

	static void printArray(const char *buf, size_t dataSize);
	void showAllConnections();

	virtual int open(void * = 0);
	virtual int handle_input(ACE_HANDLE handle = ACE_INVALID_HANDLE);
	virtual int handle_output(ACE_HANDLE handle = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask);
};

#endif