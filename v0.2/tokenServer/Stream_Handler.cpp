#include "Stream_Handler.h"
#include "CClientAcceptor.h"
#include "../common/encryptUtil.h"

const int Stream_Handler::blockSize = 0x10;
const int Stream_Handler::NumBlock = 10;

Stream_Handler::Stream_Handler()
: noti_(0, this, ACE_Event_Handler::WRITE_MASK), autheProcess_(false)
#ifdef USE_SOFTHSM
	, pHsm_(NULL), hTagKey_(0), hSeKey_(0)
#elif defined(USE_OPTEE)
	, pO_(NULL), encOp_(0), decOp_(0)
#endif
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
#ifdef USE_SOFTHSM
	pHsm_ = ca->pHsm_;
	hTagKey_ = ca->hTagKey_;
	hSeKey_ = ca->hSeKey_;
#elif defined(USE_OPTEE)
	pO_ = ca->pO_;
	encOp_ = ca->encOp_;
	decOp_ = ca->decOp_;
#endif
	return 0;
}

int Stream_Handler::handle_input(ACE_HANDLE handle)
{
	(void)handle;
	//ACE_DEBUG((LM_INFO, "(%P|%t) Stream_Handler::handle_input start\n"));
	const int bufferSize = blockSize*NumBlock;
	char *buf = new char[bufferSize + 1];	//+1:NULL을 삽입하기 위해서

	ssize_t recv_cnt;
	if ((recv_cnt = this->peer().recv(buf, bufferSize)) <= 0) {
		return -1;
	}
	buf[recv_cnt] = 0;
	ACE_DEBUG((LM_INFO, "(%P|%t) %d bytes received\n", recv_cnt));
	ACE_DEBUG((LM_INFO, "Encrypted stream:%s\n", buf));

	unsigned long ulDataLen;
	std::vector<unsigned char> vDecryptedData;
#ifdef USE_SOFTHSM
	decrypt(*pHsm_,CHsmProxy::AES_ECB, hTagKey_, (unsigned char*)buf, (unsigned long)recv_cnt, vDecryptedData, ulDataLen);
#elif defined(USE_OPTEE)
	decrypt(pO_,decOp_,(unsigned char*)buf, (unsigned long)recv_cnt, vDecryptedData, ulDataLen);
#endif
	ACE_DEBUG((LM_INFO, "Decrypt stream:%s\n", &vDecryptedData.front()));

	if (!autheProcess_) {
		autheProcess_ = true;
		std::string str = (char*)&vDecryptedData.front();
		if (str == "AuthRequest") {	//맨처음 수신한 스트림은 정상적으로 디코딩이 된다면 "AuthRequest"메세지를 받아야 한다. 그렇지 않으면 키가 달라 복호화에 실패한것임
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
#ifdef USE_SOFTHSM
	encrypt(*pHsm_,CHsmProxy::AES_ECB, hTagKey_, (unsigned char*)buf, bufferSize, vEncryptedData, ulEncryptedDataLen);
#elif defined(USE_OPTEE)
	encrypt(pO_,encOp_,(unsigned char*)buf, bufferSize, vEncryptedData, ulEncryptedDataLen);
#endif

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
	(void)handle;
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
#ifdef USE_SOFTHSM
	encrypt(*pHsm_,CHsmProxy::AES_ECB, hTagKey_, data, dataLen, vEncryptedData, ulEncryptedDataLen);
#elif defined(USE_OPTEE)
	encrypt(pO_,encOp_,data, dataLen, vEncryptedData, ulEncryptedDataLen);
#endif

	ACE_Message_Block *mb;
	ACE_NEW_RETURN(mb, ACE_Message_Block(ulEncryptedDataLen), -1);
	mb->copy((char*)&vEncryptedData.front(), ulEncryptedDataLen);
	this->putq(mb);
	return 0;
}
