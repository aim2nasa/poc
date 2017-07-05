#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <ace/SSL/SSL_Context.h>
#include <ace/INET_Addr.h>
#include <ace/Acceptor.h>
#include <ace/Message_Block.h>
#include <ace/Svc_Handler.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>
#include <ace/Synch_Traits.h>
#include <ace/Reactor.h>
#include <ace/Reactor_Notification_Strategy.h>

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
		ACE_DEBUG((LM_INFO, "(%t) Stream_Handler::handle_input start\n"));
		char buf[1024];
		ssize_t recv_cnt;
		if ((recv_cnt = this->peer().recv(buf, 1024)) <= 0) {
			return -1;
		}
		ACE_DEBUG((LM_INFO, "(%t) Stream_Handler::handle_input received(%d)\n", recv_cnt));

		ACE_Message_Block *mb;
		ACE_NEW_RETURN(mb, ACE_Message_Block(recv_cnt), -1);
		mb->copy(buf, recv_cnt);
		this->putq(mb);
		ACE_DEBUG((LM_INFO, "(%t) Stream_Handler::handle_input end\n"));
		return 0;
	}

	//override
	virtual int handle_output(ACE_HANDLE handle = ACE_INVALID_HANDLE)
	{
		ACE_DEBUG((LM_INFO, "(%t) Stream_Handler::handle_output start\n"));
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
				ACE_DEBUG((LM_INFO, "(%t) Stream_Handler::handle_output sent(%d)\n", send_cnt));
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

		ACE_DEBUG((LM_INFO, "(%t) Stream_Handler::handle_output end\n"));
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

#define SERVER_PORT 9088

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	ACE_SSL_Context *context = ACE_SSL_Context::instance();

	context->certificate("./rootcert.pem", SSL_FILETYPE_PEM);
	context->private_key("./rootkey.pem", SSL_FILETYPE_PEM);

	// 읽은 인증서와 개인키가 맞는지 확인 한다.
	if(context->verify_private_key()!=0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("인증서와 개인키가 맞지 않습니다.\n")), -1);

	ACE_INET_Addr listen;
	listen.set(SERVER_PORT);
	ACE_Acceptor<Stream_Handler, ACE_SOCK_ACCEPTOR> acceptor;
	acceptor.open(listen);
	ACE_Reactor::instance()->run_reactor_event_loop();

	ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) SSL server end\n")));
	ACE_RETURN(0);
}