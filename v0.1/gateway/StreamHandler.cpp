#include "StreamHandler.h"
#include "CSeAcceptor.h"

CID StreamHandler::sCounter_ = 0;

StreamHandler::StreamHandler()
:noti_(0, this, ACE_Event_Handler::WRITE_MASK), id_(++sCounter_)
{

}

int StreamHandler::open(void *p)
{
	ACE_TRACE("StreamHandler::open");
	if (super::open() == -1)
		return -1;

	noti_.reactor(this->reactor());
	this->msg_queue()->notification_strategy(&noti_);
	if (this->peer().get_remote_addr(remote_addr_) == 0)
	{
		ACE_DEBUG((LM_INFO, "New client accepted: %s:%u\n",
			remote_addr_.get_host_addr(), remote_addr_.get_port_number()));
	}

	CSeAcceptor *pAcceptor = static_cast<CSeAcceptor*>(p);
	pAcceptor->data.con_.insert(std::pair<CID, StreamHandler*>(id_,this));
	return 0;
}

int StreamHandler::handle_input(ACE_HANDLE handle)
{
	ACE_DEBUG((LM_INFO, "(%t) StreamHandler::handle_input start\n"));
	char buf[1024];
	ssize_t recv_cnt;
	if ((recv_cnt = this->peer().recv(buf, 1024)) <= 0) {
		return -1;
	}
	ACE_DEBUG((LM_INFO, "(%t) StreamHandler::handle_input received(%d)\n", recv_cnt));

	ACE_Message_Block *mb;
	ACE_NEW_RETURN(mb, ACE_Message_Block(recv_cnt), -1);
	mb->copy(buf, recv_cnt);
	this->putq(mb);
	ACE_DEBUG((LM_INFO, "(%t) StreamHandler::handle_input end\n"));
	return 0;
}

int StreamHandler::handle_output(ACE_HANDLE handle)
{
	ACE_DEBUG((LM_INFO, "(%t) StreamHandler::handle_output start\n"));
	ACE_Message_Block *mb;
	ACE_Time_Value nowait(ACE_OS::gettimeofday());
	while (this->getq(mb, &nowait) != -1)
	{
		ssize_t send_cnt = this->peer().send(mb->rd_ptr(), mb->length());
		if (send_cnt == -1)
			ACE_ERROR((LM_ERROR, "[ERROR%T](%N:%l) ### %p\n",
			"StreamHandler::handle_output"));
		else {
			mb->rd_ptr(send_cnt);
			ACE_DEBUG((LM_INFO, "(%t) StreamHandler::handle_output sent(%d)\n", send_cnt));
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

	ACE_DEBUG((LM_INFO, "(%t) StreamHandler::handle_output end\n"));
	return 0;
}

int StreamHandler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
	ACE_TRACE("StreamHandler::handle_close");
	ACE_DEBUG((LM_INFO, "Connection close %s:%u\n", remote_addr_.get_host_addr(), remote_addr_.get_port_number()));
	return super::handle_close(handle, close_mask);
}

CID StreamHandler::id()
{
	return id_;
}