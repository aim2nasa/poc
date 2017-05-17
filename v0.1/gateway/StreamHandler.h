#ifndef __STREAMHANDLER_H__
#define __STREAMHANDLER_H__

#include <ace/Svc_Handler.h>
#include <ace/SOCK_Stream.h>
#include <ace/Reactor_Notification_Strategy.h>

class StreamHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> {
private:
	typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;

	ACE_INET_Addr remote_addr_;
	ACE_Reactor_Notification_Strategy noti_;

public:
	StreamHandler();

	virtual int open(void * = 0);
	virtual int handle_input(ACE_HANDLE handle = ACE_INVALID_HANDLE);
	virtual int handle_output(ACE_HANDLE handle = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask);
};

#endif