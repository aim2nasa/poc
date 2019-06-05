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

std::string encrypt(size_t keySize,int key,int iv,int tagSize,std::string& adata,std::string message)
{
    Node Alice;
    Alice.size_ = keySize;
    Alice.key_ = new byte[Alice.size_];
    memset(Alice.key_,key,Alice.size_);
    memset(Alice.iv_,iv,CryptoPP::AES::BLOCKSIZE);

    CryptoPP::GCM<CryptoPP::AES>::Encryption e;
    e.SetKeyWithIV(Alice.key_,Alice.size_,Alice.iv_);
    return Alice.encrypt(e,adata,message,tagSize);
}

TEST(Classifier, ask)
{
	std::string adata(16,(char)0x00);
	std::string cipherText = encrypt(/*keySize*/32,/*key*/3,/*iv*/4,/*tagSize*/16,adata,"I love you, Bob");
	ASSERT_NE("I love you, Bob",cipherText);

	Classifier cf;
	cf.init(/*tagSize*/16,adata,/*keySize*/32,/*key*/3,/*iv*/4);

	struct messageCount msg;
	msg.size = cipherText.size();
	memcpy(msg.body,cipherText.c_str(),msg.size);
	ASSERT_EQ(msg.visitCount,0);
	cf.q_.push_back(msg);

	ASSERT_EQ(cf.q_.size(),1);
	ASSERT_EQ(cf.q_.front().visitCount,0);
	ASSERT_EQ(cf.ask(cipherText.c_str(),msg.size),Classifier::verified);
	ASSERT_EQ(cf.q_.front().visitCount,1);
	ASSERT_EQ(cf.ask(cipherText.c_str(),msg.size),Classifier::replay);
	ASSERT_EQ(cf.q_.front().visitCount,2);
}
