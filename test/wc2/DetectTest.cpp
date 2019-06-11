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

std::string encrypt(size_t keySize,int key,int iv,int tagSize,std::string& adata,std::string message,size_t messageSize)
{
    Node Alice;
    Alice.size_ = keySize;
    Alice.key_ = new byte[Alice.size_];
    memset(Alice.key_,key,Alice.size_);
    memset(Alice.iv_,iv,CryptoPP::AES::BLOCKSIZE);

    CryptoPP::GCM<CryptoPP::AES>::Encryption e;
    e.SetKeyWithIV(Alice.key_,Alice.size_,Alice.iv_);
	return Alice.encrypt(e,adata,reinterpret_cast<const byte*>(message.c_str()),messageSize,tagSize);
}

std::string encrypt(size_t keySize,int key,int iv,int tagSize,std::string& adata,std::string message)
{
	return encrypt(keySize,key,iv,tagSize,adata,message,message.size());
}

void msgToTransmittedQueue(std::vector<messageCount>& q,std::string& cipherText)
{
	struct messageCount msg;
	msg.size = cipherText.size();
	memcpy(msg.body,cipherText.c_str(),msg.size);
	ASSERT_EQ(msg.visitCount,0);
	q.push_back(msg);
}

std::string firstVerifiedData(size_t keySize,int key,int iv,int tagSize,std::string& adata,std::string message,
							  Classifier& cf)
{
	std::string cipherText = encrypt(keySize,key,iv,tagSize,adata,message);
	cf.init(tagSize,adata,keySize,key,iv);
	msgToTransmittedQueue(cf.q_,cipherText);
	return cipherText;
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
