#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <ace/Message_Block.h>
#include <ace/INET_Addr.h>
#include <ace/Svc_Handler.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>
#include <ace/Synch_Traits.h>
#include <ace/Reactor.h>
#include <ace/Acceptor.h>
#include <ace/Reactor_Notification_Strategy.h>
#include "CHsmProxy.h"
#include "testConf.h"

CHsmProxy hsm;	//전역변수

class Stream_Handler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> {
private:
	typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;

	ACE_INET_Addr remote_addr_;
	ACE_Reactor_Notification_Strategy noti_;

public:
	Stream_Handler()
		: noti_(0, this, ACE_Event_Handler::WRITE_MASK)
	{ /* empty */
	}

	//override
	virtual int open(void * = 0)
	{
		ACE_TRACE("Stream_Handler::open");
		if (super::open() == -1)
			return -1;
		noti_.reactor(this->reactor());
		this->msg_queue()->notification_strategy(&noti_);
		if (this->peer().get_remote_addr(remote_addr_) == 0)
		{
			ACE_DEBUG((LM_INFO, "New client accepted: %s:%u\n",
				remote_addr_.get_host_addr(), remote_addr_.get_port_number()));
		}
		return 0;
	}

	//override
	virtual int handle_input(ACE_HANDLE handle = ACE_INVALID_HANDLE)
	{
		//ACE_DEBUG((LM_INFO, "(%P|%t) Stream_Handler::handle_input start\n"));
		char buf[1024];
		ssize_t recv_cnt;
		if ((recv_cnt = this->peer().recv(buf, 1024)) <= 0) {
			return -1;
		}
		buf[recv_cnt] = 0;
		ACE_DEBUG((LM_INFO, "(%P|%t) %d bytes received\n", recv_cnt));
		ACE_DEBUG((LM_INFO, "%s\n", buf));

		ACE_DEBUG((LM_INFO, ": "));
		fgets(buf, sizeof(buf), stdin);

		ACE_Message_Block *mb;
		ACE_NEW_RETURN(mb, ACE_Message_Block(ACE_OS::strlen(buf)), -1);
		mb->copy(buf, ACE_OS::strlen(buf));
		this->putq(mb);
		//ACE_DEBUG((LM_INFO, "(%P|%t) Stream_Handler::handle_input end\n"));
		return 0;
	}

	//override
	virtual int handle_output(ACE_HANDLE handle = ACE_INVALID_HANDLE)
	{
		//ACE_DEBUG((LM_INFO, "(%t) Stream_Handler::handle_output start\n"));
		ACE_Message_Block *mb;
		ACE_Time_Value nowait(ACE_OS::gettimeofday());
		while (this->getq(mb, &nowait) != -1)
		{
			ssize_t send_cnt = this->peer().send(mb->rd_ptr(), mb->length());
			if (send_cnt == -1)
				ACE_ERROR((LM_ERROR, "[ERROR%T](%N:%l) ### %p\n",
				"Stream_Handler::handle_output"));
			else {
				mb->rd_ptr(send_cnt);
				ACE_DEBUG((LM_INFO, "(%P|%t) %d bytes sent\n", send_cnt));
			}

			if (mb->length() > 0)
			{
				this->ungetq(mb);
				break;
			}
			mb->release();
		}
		if (this->msg_queue()->is_empty())
			this->reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
		else
			this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);

		//ACE_DEBUG((LM_INFO, "(%t) Stream_Handler::handle_output end\n"));
		return 0;
	}

	//override
	virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
	{
		ACE_TRACE("Stream_Handler::handle_close");
		ACE_DEBUG((LM_INFO, "Connection close %s:%u\n", remote_addr_.get_host_addr(), remote_addr_.get_port_number()));
		return super::handle_close(handle, close_mask);
	}

};

#define SERVER_PORT 9870

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	u_short server_port = argc > 1 ? ACE_OS::atoi(argv[1]) : SERVER_PORT;
	ACE_DEBUG((LM_INFO, "(%t) server start at port:%d\n", server_port));

	ACE_INET_Addr listen;
	listen.set(SERVER_PORT);
	ACE_Acceptor<Stream_Handler, ACE_SOCK_ACCEPTOR> acceptor;
	acceptor.open(listen);

	hsm.setenv("SOFTHSM2_CONF", ".\\se1Token.conf", 1);
	ACE_ASSERT(hsm.init(SO_PIN, USER_PIN) == 0);

	ACE_Reactor::instance()->run_reactor_event_loop();

	ACE_DEBUG((LM_INFO, "(%t) server end\n"));
	ACE_RETURN(0);
}