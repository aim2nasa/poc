#include "StreamHandler.h"
#include "CGwData.h"
#include "protocol.h"
#include "CCtrlProxy.h"
#include "sndRcv.h"

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
	CGwData::getInstance()->con_.insert(std::pair<CID, StreamHandler*>(id_, this));
	return 0;
}

int StreamHandler::handle_input(ACE_HANDLE handle)
{
	ACE_DEBUG((LM_INFO, "(%t) StreamHandler::handle_input\n"));

	char buf[1024];
	ssize_t recv_cnt;

	//prefix
	recv_cnt = rcv(this->peer(), buf, PREFIX_SIZE);
	
	if (recv_cnt==0)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) peer closed connection\n")), -1);
	else if (recv_cnt != PREFIX_SIZE)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) wrong prefix size(%d)\n"), recv_cnt), -1);

	ACE_ASSERT(PREFIX_SIZE == recv_cnt);

	buf[PREFIX_SIZE] = 0;
	std::string prefix = buf;

	//dataSize
	ACE_INT32 len;
	recv_cnt = rcv(this->peer(), &len, sizeof(ACE_INT32));
	ACE_INT32 dataSize = ACE_NTOHL(len);

	//data
	recv_cnt = rcv(this->peer(), buf, dataSize);
	
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

	std::map<CID, StreamHandler*>::iterator it;
	it = CGwData::getInstance()->con_.find(id_);
	ACE_ASSERT(it != CGwData::getInstance()->con_.end());
	CGwData::getInstance()->con_.erase(it);
	showAllConnections();

	ACE_DEBUG((LM_INFO, "Connection close %s:%u\n", remote_addr_.get_host_addr(), remote_addr_.get_port_number()));
	return super::handle_close(handle, close_mask);
}

CID StreamHandler::id()
{
	return id_;
}

char* StreamHandler::serialNo()
{
	return serialNo_;
}

int StreamHandler::onSerialNo(const char *buf, size_t dataSize)
{
	ACE_ASSERT(dataSize == SERIAL_NO_SIZE);
	memcpy(serialNo_, buf, dataSize);
	showAllConnections();
	if (CGwData::getInstance()->ctrlProxy_) CGwData::getInstance()->ctrlProxy_->onReqStat();
	return 0;
}

void StreamHandler::printArray(const char *buf, size_t dataSize)
{
	for (size_t i = 0; i < dataSize; i++)
		ACE_DEBUG((LM_INFO, "%0x ", static_cast<unsigned char>(buf[i])));
}

void StreamHandler::showAllConnections()
{
	ACE_DEBUG((LM_INFO, "* List of all connections\n"));
	for (std::map<CID, StreamHandler*>::iterator it = CGwData::getInstance()->con_.begin(); it != CGwData::getInstance()->con_.end(); ++it) {
		ACE_DEBUG((LM_INFO, " CID:%u,SerialNo:", it->first));
		printArray(it->second->serialNo(), SERIAL_NO_SIZE);
		ACE_DEBUG((LM_INFO, "\n"));
	}
}