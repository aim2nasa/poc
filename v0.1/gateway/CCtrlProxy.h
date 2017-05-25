#ifndef __CCTRLPROXY_H__
#define __CCTRLPROXY_H__

#include <ace/Svc_Handler.h>
#include <ace/SOCK_Stream.h>
#include <ace/Reactor_Notification_Strategy.h>

class CGroup;

class CCtrlProxy : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> {
private:
	typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;

	ACE_INET_Addr remote_addr_;
	ACE_Reactor_Notification_Strategy noti_;

public:
	CCtrlProxy();
	int onReqStat();
	int onReqKeyG(const char *buf, size_t dataSize);
	int generateKey(CGroup &group);

	virtual int open(void * = 0);
	virtual int handle_input(ACE_HANDLE handle = ACE_INVALID_HANDLE);
	virtual int handle_output(ACE_HANDLE handle = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask);
};

#endif