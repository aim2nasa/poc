#include "Stream_Handler.h"
#include "CClientAcceptor.h"

Stream_Handler::Stream_Handler()
: noti_(0, this, ACE_Event_Handler::WRITE_MASK), autheProcess_(false), pHsm_(NULL), hTagKey_(0), hSeKey_(0)
{

}

int Stream_Handler::open(void * p)
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
	CClientAcceptor* ca = ((CClientAcceptor*)p);
	pHsm_ = ca->pHsm_;
	hTagKey_ = ca->hTagKey_;
	hSeKey_ = ca->hSeKey_;
	return 0;
}

int Stream_Handler::handle_input(ACE_HANDLE handle)
{
	//ACE_DEBUG((LM_INFO, "(%P|%t) Stream_Handler::handle_input start\n"));
	const int blockSize(0x10);
	const int NumBlock(10);
	const int bufferSize = blockSize*NumBlock;
	char *buf = new char[bufferSize + 1];	//+1:NULL�� �����ϱ� ���ؼ�

	ssize_t recv_cnt;
	if ((recv_cnt = this->peer().recv(buf, bufferSize)) <= 0) {
		return -1;
	}
	buf[recv_cnt] = 0;
	ACE_DEBUG((LM_INFO, "(%P|%t) %d bytes received\n", recv_cnt));
	ACE_DEBUG((LM_INFO, "Encrypted stream:%s\n", buf));

	unsigned long ulDataLen;
	std::vector<unsigned char> vDecryptedData;
	decrypt(CHsmProxy::AES_ECB, hTagKey_, (unsigned char*)buf, (unsigned long)recv_cnt, vDecryptedData, ulDataLen);
	ACE_DEBUG((LM_INFO, "Decrypt stream:%s\n", &vDecryptedData.front()));

	if (!autheProcess_) {
		autheProcess_ = true;
		std::string str = (char*)&vDecryptedData.front();
		if (str == "AuthRequest") {	//��ó�� ������ ��Ʈ���� ���������� ���ڵ��� �ȴٸ� "AuthRequest"�޼����� �޾ƾ� �Ѵ�. �׷��� ������ Ű�� �޶� ��ȣȭ�� �����Ѱ���
			str += ":Done";
			ACE_OS::memset(buf, 0, bufferSize + 1);
			ACE_OS::memcpy(buf, str.c_str(), str.size());
			sendAuthRequestResult((unsigned char*)buf, bufferSize);
			ACE_DEBUG((LM_INFO, "Client Authenticated\n"));
			return 0;
		}
		else{
			ACE_DEBUG((LM_INFO, "Client Authentication failed\n"));
			return -1;
		}
	}

	ACE_DEBUG((LM_INFO, ": "));
	fgets(buf, bufferSize, stdin);

	unsigned long ulEncryptedDataLen;
	std::vector<unsigned char> vEncryptedData;
	encrypt(CHsmProxy::AES_ECB, hTagKey_, (unsigned char*)buf, bufferSize, vEncryptedData, ulEncryptedDataLen);

	ACE_Message_Block *mb;
	ACE_NEW_RETURN(mb, ACE_Message_Block(ulEncryptedDataLen), -1);
	mb->copy((char*)&vEncryptedData.front(), ulEncryptedDataLen);
	this->putq(mb);

	delete[] buf;
	//ACE_DEBUG((LM_INFO, "(%P|%t) Stream_Handler::handle_input end\n"));
	return 0;
}

int Stream_Handler::handle_output(ACE_HANDLE handle)
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

int Stream_Handler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
	ACE_TRACE("Stream_Handler::handle_close");
	ACE_DEBUG((LM_INFO, "Connection close %s:%u\n", remote_addr_.get_host_addr(), remote_addr_.get_port_number()));
	return super::handle_close(handle, close_mask);
}

int Stream_Handler::sendAuthRequestResult(unsigned char *data, unsigned long dataLen)
{
	unsigned long ulEncryptedDataLen;
	std::vector<unsigned char> vEncryptedData;
	encrypt(CHsmProxy::AES_ECB, hTagKey_, data, dataLen, vEncryptedData, ulEncryptedDataLen);

	ACE_Message_Block *mb;
	ACE_NEW_RETURN(mb, ACE_Message_Block(ulEncryptedDataLen), -1);
	mb->copy((char*)&vEncryptedData.front(), ulEncryptedDataLen);
	this->putq(mb);
	return 0;
}

void Stream_Handler::encrypt(CHsmProxy::MechanismType mType, unsigned long hKey, unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vEncryptedData, unsigned long &ulEncryptedDataLen)
{
	CHsmProxy &hsm = *pHsm_;
	ACE_ASSERT(hsm.encryptInit(mType, hKey) == 0);
	ACE_ASSERT(hsm.encrypt(data, dataLen, NULL, &ulEncryptedDataLen) == 0);
	ACE_ASSERT(ulEncryptedDataLen == dataLen);

	vEncryptedData.resize(ulEncryptedDataLen);
	ACE_ASSERT(hsm.encrypt(data, dataLen, &vEncryptedData.front(), &ulEncryptedDataLen) == 0);
	ACE_ASSERT(ulEncryptedDataLen == dataLen);
}

void Stream_Handler::decrypt(CHsmProxy::MechanismType mType, unsigned long hKey, unsigned char *data, unsigned long dataLen, std::vector<unsigned char> &vDecryptedData, unsigned long &ulDecryptedDataLen)
{
	CHsmProxy &hsm = *pHsm_;
	ACE_ASSERT(hsm.decryptInit(mType, hKey) == 0);
	ACE_ASSERT(hsm.decrypt(data, dataLen, NULL, &ulDecryptedDataLen) == 0);

	vDecryptedData.resize(ulDecryptedDataLen);
	ACE_ASSERT(hsm.decrypt(data, dataLen, &vDecryptedData.front(), &ulDecryptedDataLen) == 0);
}