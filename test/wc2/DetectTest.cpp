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

TEST(Classifier, ask)
{
	int kv = 3; //ranom value
	int ivv = 4; //ranom value
	int tagSize = 16;
	std::string adata(16,(char)0x00);

	//Alice
	Node Alice;
	Alice.size_=32;
	Alice.key_ = new byte[Alice.size_];
	memset(Alice.key_,kv,Alice.size_);
	memset(Alice.iv_,ivv,CryptoPP::AES::BLOCKSIZE);

	//Secret message fron Alice to Bob
	//Encryption from Alice
	std::string message = "I love you, Bob";
	CryptoPP::GCM<CryptoPP::AES>::Encryption e;
	e.SetKeyWithIV(Alice.key_,Alice.size_,Alice.iv_);
	std::string cipherText = Alice.encrypt(e,adata,message,tagSize);
	ASSERT_NE(message,cipherText);

	Classifier cf;
	cf.init(tagSize,adata,Alice.size_,kv,ivv);

	struct messageCount msg;
	msg.size = cipherText.size();
	memcpy(msg.body,cipherText.c_str(),msg.size);
	cf.q_.push_back(msg);

	ASSERT_EQ(cf.q_.size(),1);
	ASSERT_EQ(cf.ask(cipherText.c_str(),msg.size),Classifier::verified);
}
