#include "CCtrlProxy.h"
#include "CGwData.h"
#include "CGroup.h"

CCtrlProxy::CCtrlProxy()
:noti_(0, this, ACE_Event_Handler::WRITE_MASK)
{

}

int CCtrlProxy::open(void *)
{
	ACE_TRACE("CCtrlProxy::open");
	ACE_DEBUG((LM_INFO, "(%t) CCtrlProxy::open\n"));
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

int CCtrlProxy::handle_input(ACE_HANDLE handle)
{
	ACE_DEBUG((LM_INFO, "(%t) CCtrlProxy::handle_input\n"));

	char buf[1024];
	ssize_t recv_cnt;

	//prefix
	if ((recv_cnt = this->peer().recv_n(buf, PREFIX_SIZE)) <= 0)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "CCtrlProxy, prefix receive error (%d)", recv_cnt), -1);

	ACE_ASSERT(PREFIX_SIZE == recv_cnt);

	buf[PREFIX_SIZE] = 0;
	std::string prefix = buf;

	//dataSize
	ACE_INT32 len;
	if ((recv_cnt = this->peer().recv_n(&len, sizeof(ACE_INT32))) <= 0)
		ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "CCtrlProxy, dataSize receive error (%d)", recv_cnt), -1);
	ACE_INT32 dataSize = len;

	//data
	if (dataSize > 0) {
		if ((recv_cnt = this->peer().recv_n(buf, dataSize)) <= 0)
			ACE_ERROR_RETURN((LM_ERROR, "(%P|%t) %p \n", "data receive error (%d)", recv_cnt), -1);

		ACE_ASSERT(dataSize == recv_cnt);
	}

	if (prefix == PRF_REQ_STAT) onReqStat();
	if (prefix == PRF_REQ_KEYG) onReqKeyG(buf,dataSize);
	return 0;
}

int CCtrlProxy::handle_output(ACE_HANDLE handle)
{
	ACE_TRACE("CCtrlProxy::handle_output");
	ACE_DEBUG((LM_INFO, "(%t) CCtrlProxy::handle_output\n"));
	ACE_Message_Block *mb;
	ACE_Time_Value nowait(ACE_OS::gettimeofday());
	while (this->getq(mb, &nowait) != -1)
	{
		ssize_t send_cnt = this->peer().send(mb->rd_ptr(), mb->length());
		if (send_cnt == -1)
			ACE_ERROR((LM_ERROR, "[ERROR%T](%N:%l) ### %p\n","CCtrlProxy::handle_output"));
		else {
			mb->rd_ptr(send_cnt);
			ACE_DEBUG((LM_INFO, "(%t) CCtrlProxy::handle_output sent(%d)\n", send_cnt));
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
	return 0;
}

int CCtrlProxy::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
	ACE_TRACE("CCtrlProxy::handle_close");
	ACE_DEBUG((LM_INFO, "(%t) CCtrlProxy::handle_close\n"));
	ACE_DEBUG((LM_INFO, "Connection close %s:%u\n", remote_addr_.get_host_addr(), remote_addr_.get_port_number()));
	return super::handle_close(handle, close_mask);
}

int CCtrlProxy::onReqStat()
{
	//{prefix, 8바이트} {dataSize,바이트} {data}

	//ACK_STAT data
	ACE_Message_Block *mb;
	ACE_UINT32 dataSize = CGwData::getInstance()->con_.size()*(sizeof(ACE_UINT32)+SERIAL_NO_SIZE);
	ACE_NEW_RETURN(mb, ACE_Message_Block(PREFIX_SIZE + sizeof(ACE_UINT32) + dataSize), -1);

	ACE_OS::memcpy(mb->wr_ptr(), PRF_ACK_STAT, PREFIX_SIZE);
	mb->wr_ptr(PREFIX_SIZE);

	ACE_OS::memcpy(mb->wr_ptr(), &dataSize, sizeof(ACE_UINT32));
	mb->wr_ptr(sizeof(ACE_UINT32));

	for (std::map<CID, StreamHandler*>::iterator it = CGwData::getInstance()->con_.begin(); it != CGwData::getInstance()->con_.end(); ++it) {
		ACE_UINT32 cid = it->first;
		ACE_OS::memcpy(mb->wr_ptr(), &cid, sizeof(ACE_UINT32));
		mb->wr_ptr(sizeof(ACE_UINT32));

		ACE_OS::memcpy(mb->wr_ptr(), it->second->serialNo(), SERIAL_NO_SIZE);
		mb->wr_ptr(SERIAL_NO_SIZE);
	}
	return this->putq(mb);
}

int CCtrlProxy::onReqKeyG(const char *buf, size_t dataSize)
{
	ACE_DEBUG((LM_INFO, "(%t) CCtrlProxy::onReqKeyG "));

	CGroup group;
	//GroupName
	char groupName[GROUP_NAME_SIZE];
	ACE_OS::memcpy(groupName, buf, GROUP_NAME_SIZE);
	ACE_DEBUG((LM_INFO, ACE_TEXT("GroupName:%s ["), groupName));
	group.groupName_ = groupName;

	//CIDs
	size_t count = (dataSize-GROUP_NAME_SIZE) / sizeof(ACE_UINT32);
	buf += GROUP_NAME_SIZE;
	for (size_t i = 0; i < count; i++) {
		ACE_UINT32 cid;
		ACE_OS::memcpy(&cid, buf + i*sizeof(ACE_UINT32), sizeof(ACE_UINT32));
		ACE_DEBUG((LM_INFO, "%d ",cid));
		group.cidList_.push_back(cid);
	}
	ACE_DEBUG((LM_INFO, "]\n"));
	generateKey(group);
	return 0;
}

int CCtrlProxy::generateKey(CGroup &group)
{
	ACE_DEBUG((LM_INFO, "(%t) CCtrlProxy::generateKey\n"));
	return 0;
}