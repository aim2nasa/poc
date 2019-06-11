#include "gmock/gmock.h"
#include "src/Detect.h"
#include "Classifier.h"

TEST(Detect, run)
{
    Detect d;
    d.start(&d);
    d.stop();
    d.join();
}

TEST(Detect, messages)
{
    Detect d;
    d.start(&d);

    char msg[128];
    for(int i=0;i<5;i++) {
        sprintf(msg,"Message #%d",i);
        d.recv((void*)msg,strlen(msg));
    }

    d.stop();
    d.join();
}

std::string encrypt(size_t keySize,int key,int iv,int tagSize,std::string& adata,const byte* message,size_t messageSize)
{
    Node Alice;
    Alice.size_ = keySize;
    Alice.key_ = new byte[Alice.size_];
    memset(Alice.key_,key,Alice.size_);
    memset(Alice.iv_,iv,CryptoPP::AES::BLOCKSIZE);

    CryptoPP::GCM<CryptoPP::AES>::Encryption e;
    e.SetKeyWithIV(Alice.key_,Alice.size_,Alice.iv_);
	return Alice.encrypt(e,adata,message,messageSize,tagSize);
}

std::string encrypt(size_t keySize,int key,int iv,int tagSize,std::string& adata,std::string message)
{
	return encrypt(keySize,key,iv,tagSize,adata,reinterpret_cast<const byte*>(message.c_str()),message.size());
}

std::string decrypt(size_t keySize,int key,int iv,int tagSize,std::string& adata,std::string cipherText)
{
    Node Bob;
    Bob.size_ = keySize;
    Bob.key_ = new byte[Bob.size_];
    memset(Bob.key_,key,Bob.size_);
    memset(Bob.iv_,iv,CryptoPP::AES::BLOCKSIZE);

    CryptoPP::GCM<CryptoPP::AES>::Decryption d;
    d.SetKeyWithIV(Bob.key_,Bob.size_,Bob.iv_);
	std::string recoveredText;
	if(DECRYPT_OK==Bob.decrypt(d,tagSize,adata,cipherText,recoveredText)) return recoveredText;
	return "";
}

void msgToTransmittedQueue(std::vector<messageCount>& q,std::string& cipherText)
{
	struct messageCount msg;
	msg.size = cipherText.size();
	memcpy(msg.body,cipherText.c_str(),msg.size);
	ASSERT_EQ(msg.visitCount,0);
	q.push_back(msg);
}

std::string firstVerifiedData(size_t keySize,int key,int iv,int tagSize,std::string& adata,const byte* message,
							  size_t messageSize,Classifier& cf)
{
	std::string cipherText = encrypt(keySize,key,iv,tagSize,adata,message,messageSize);
	cf.init(tagSize,adata,keySize,key,iv);
	msgToTransmittedQueue(cf.q_,cipherText);
	return cipherText;
}

std::string firstVerifiedData(size_t keySize,int key,int iv,int tagSize,std::string& adata,std::string message,
							  Classifier& cf)
{
	return firstVerifiedData(keySize,key,iv,tagSize,adata,reinterpret_cast<const byte*>(message.c_str()),message.size(),cf);
}

TEST(Classifier, ask_onFirstVerifiedData)
{
	Classifier cf;
	std::string adata(16,(char)0x00);
	std::string cipherText = firstVerifiedData(/*keySize*/32,/*key*/3,/*iv*/4,/*tagSize*/16,adata,"I love you, Bob",cf);
	ASSERT_NE("I love you, Bob",cipherText);

	ASSERT_EQ(cf.q_.size(),1);
	ASSERT_EQ(cf.q_.front().visitCount,0);
	ASSERT_EQ(cf.ask(cipherText.c_str(),cipherText.size()),Classifier::verified);
	ASSERT_EQ(cf.q_.front().visitCount,1);
}

TEST(Classifier, ask_UnauthorizedPublishAttack)
{
	char unAuthPub[256]={0};	//represent random data which can't be decrypted with key,iv of Classifier

	std::string adata(16,(char)0x00);
	Classifier cf;
	cf.init(/*tagSize*/16,adata,/*keySize*/32,/*key*/3,/*iv*/4);

	ASSERT_EQ(cf.q_.size(),0);
	ASSERT_EQ(cf.ask(unAuthPub,sizeof(unAuthPub)),Classifier::unAuthPub);
}

TEST(Classifier, ask_Fraud)
{
	std::string adata(16,(char)0x00);
	std::string cipherText = encrypt(/*keySize*/32,/*key*/3,/*iv*/4,/*tagSize*/16,adata,"I love you, Bob");
	ASSERT_NE("I love you, Bob",cipherText);

	Classifier cf;
	cf.init(/*tagSize*/16,adata,/*keySize*/32,/*key*/3,/*iv*/4);

	ASSERT_EQ(cf.q_.size(),0);
	ASSERT_EQ(cf.ask(cipherText.c_str(),cipherText.size()),Classifier::fraud);
}

TEST(Classifier, ask_Replay_NoQueueOverflow)
{
	Classifier cf;
	std::string adata(16,(char)0x00);
	std::string cipherText = firstVerifiedData(/*keySize*/32,/*key*/3,/*iv*/4,/*tagSize*/16,adata,"I love you, Bob",cf);
	ASSERT_NE("I love you, Bob",cipherText);

	ASSERT_EQ(cf.q_.size(),1);
	ASSERT_EQ(cf.q_.front().visitCount,0);
	ASSERT_EQ(cf.ask(cipherText.c_str(),cipherText.size()),Classifier::verified);
	ASSERT_EQ(cf.q_.front().visitCount,1);

	for(int i=0;i<100;i++) {
		ASSERT_EQ(cf.ask(cipherText.c_str(),cipherText.size()),Classifier::replay);
		ASSERT_EQ(cf.q_.front().visitCount,i+2);	//visitCount increases
	}
}

TEST(Classifier, ask_Replay_Judgement)
{
	Classifier cf;
	std::string adata(16,(char)0x00);
	std::string cipherText = firstVerifiedData(/*keySize*/32,/*key*/3,/*iv*/4,/*tagSize*/16,adata,"I love you, Bob",cf);
	ASSERT_NE("I love you, Bob",cipherText);

	ASSERT_EQ(cf.q_.size(),1);
	ASSERT_EQ(cf.q_.front().visitCount,0);
	ASSERT_EQ(cf.ask(cipherText.c_str(),cipherText.size()),Classifier::verified);
	ASSERT_EQ(cf.q_.front().visitCount,1);

	//replay occurs
	ASSERT_EQ(cf.ask(cipherText.c_str(),cipherText.size()),Classifier::replay);
	ASSERT_EQ(cf.q_.front().visitCount,2);

	//clear queue then same replay attack again
	cf.q_.clear();
	ASSERT_EQ(cf.q_.size(),0);
	ASSERT_EQ(cf.ask(cipherText.c_str(),cipherText.size()),Classifier::fraud);	//when data is not found in q,regarded as fraud
}

size_t makeMessage(unsigned int sequence,const char* msg,char* buffer)
{
	sprintf(buffer,"%s-%u",msg,sequence);
	size_t bytes = strlen(buffer);
	memmove(buffer+sizeof(sequence),buffer,bytes);
	memcpy(buffer,&sequence,sizeof(sequence));
	return bytes+sizeof(sequence);
}

TEST(Classifier, ask_Sequence_Correct)
{
	Classifier cf;
	char buffer[256];

	//first frame(sequence:0)
	size_t msgSize = makeMessage(/*sequence*/0,"message",buffer);
	std::string adata(16,(char)0x00);
	std::string cipherText = firstVerifiedData(/*keySize*/32,/*key*/3,/*iv*/4,/*tagSize*/16,adata,reinterpret_cast<const byte*>(buffer),msgSize,cf);
	ASSERT_GT(cipherText.size(),0);
	std::string decMsg = decrypt(/*keySize*/32,/*key*/3,/*iv*/4,/*tagSize*/16,adata,cipherText);
	ASSERT_EQ(decMsg.size(),13);	//(sequence)+"message-0"=4+9=13
	ASSERT_EQ(decMsg.substr(4,13),"message-0");	//compare exclude the sequence, cause sequence is just hexa value
	unsigned int number;
	memcpy(&number,decMsg.c_str(),sizeof(number));
	ASSERT_EQ(number,0);	//sequence number

	ASSERT_EQ(cf.q_.size(),1);
	ASSERT_EQ(cf.q_.front().visitCount,0);
	ASSERT_EQ(cf.ask(cipherText.c_str(),cipherText.size()),Classifier::verified);
	ASSERT_EQ(cf.q_.front().visitCount,1);

	//second frame(sequence:1)
	msgSize = makeMessage(/*sequence*/1,"message",buffer);
	cipherText = encrypt(/*keySize*/32,/*key*/3,/*iv*/4,/*tagSize*/16,adata,reinterpret_cast<const byte*>(buffer),msgSize);
	ASSERT_GT(cipherText.size(),0);
	decMsg = decrypt(/*keySize*/32,/*key*/3,/*iv*/4,/*tagSize*/16,adata,cipherText);
	ASSERT_EQ(decMsg.size(),13);	//(sequence)+"message-0"=4+9=13
	ASSERT_EQ(decMsg.substr(4,13),"message-1");	//compare exclude the sequence, cause sequence is just hexa value
	memcpy(&number,decMsg.c_str(),sizeof(number));
	ASSERT_EQ(number,1);	//sequence number

	msgToTransmittedQueue(cf.q_,cipherText);

	ASSERT_EQ(cf.q_.size(),2);
	ASSERT_EQ(cf.q_.at(1).visitCount,0);
	ASSERT_EQ(cf.ask(cipherText.c_str(),cipherText.size()),Classifier::verified);
