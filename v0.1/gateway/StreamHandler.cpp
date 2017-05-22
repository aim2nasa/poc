#include "StreamHandler.h"
#include "CSeAcceptor.h"
#include "protocol.h"

CID StreamHandler::sCounter_ = 0;

StreamHandler::StreamHandler()
:noti_(0, this, ACE_Event_Handler::WRITE_MASK), id_(++sCounter_)
{
	memset(serialNo_, 0, sizeof(serialNo_));
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
	ACE_DEBUG((LM_INFO, "(%t) StreamHandler::handle_input\n"));

	char buf[1024];
	ssize_t recv_cnt;

	//prefix
	if ((recv_cnt = this->peer().recv_n(buf, PREFIX_SIZE)) <= 0)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "prefix receive error (%d)", recv_cnt), -1);

	ACE_ASSERT(PREFIX_SIZE == recv_cnt);

	buf[PREFIX_SIZE] = 0;
	std::string prefix = buf;

	//dataSize
	ACE_INT32 len;
	if ((recv_cnt = this->peer().recv_n(&len, sizeof(ACE_INT32))) <= 0)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "dataSize receive error (%d)", recv_cnt), -1);
	ACE_INT32 dataSize = ACE_NTOHL(len);

	//data
	if ((recv_cnt = this->peer().recv_n(buf, dataSize)) <= 0)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "data receive error (%d)", recv_cnt), -1);
	
	ACE_ASSERT(dataSize == recv_cnt);
	if (prefix == PRF_SERIALNO) onSerialNo(buf, dataSize);
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

unsigned char* StreamHandler::serialNo()
{
	return serialNo_;
}

int StreamHandler::onSerialNo(const char *buf, size_t dataSize)
{
	ACE_ASSERT(dataSize == SERIAL_NO_SIZE);
	ACE_DEBUG((LM_INFO, "(%t) serialNo:"));

	const unsigned char *pSn = (const unsigned char*)buf;
	memcpy(serialNo_, pSn, dataSize);
	printArray(pSn,dataSize);
	ACE_DEBUG((LM_INFO, "\n"));
	return 0;
}

void StreamHandler::printArray(const unsigned char *buf, size_t dataSize)
{
	for (size_t i = 0; i < dataSize; i++)
		ACE_DEBUG((LM_INFO, "%0x ", buf[i]));
}