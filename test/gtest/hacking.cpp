#include <gtest/gtest.h>
#include <entity/keyStore.h>
#include <entity/node.h>
 
TEST(hackingTest, unauthorised_Subscription) { 
	ASSERT_EQ(16,sizeof(Node::iv_));

	//Alice
	Node Alice;
	int kv = 3; //ranom value
	int ivv = 4; //ranom value

	Alice.size_=32;
	Alice.key_ = new byte[Alice.size_];
	memset(Alice.key_,kv,Alice.size_);
	memset(Alice.iv_,ivv,CryptoPP::AES::BLOCKSIZE);

	//Bob, Alice and Bob share the same key and iv
	Node Bob;
	Bob.size_ = Alice.size_;
	Bob.key_ = new byte[Bob.size_];
	memset(Bob.key_,kv,Bob.size_);
	memset(Bob.iv_,ivv,CryptoPP::AES::BLOCKSIZE);
	ASSERT_EQ(Alice.size_,Bob.size_);
	ASSERT_EQ(memcmp(Alice.key_,Bob.key_,Alice.size_),0);
	ASSERT_EQ(memcmp(Alice.iv_,Bob.iv_,CryptoPP::AES::BLOCKSIZE),0);

	//Secret message fron Alice to Bob
	{
		int tagSize = 16;
		//Encryption from Alice
		std::string message = "Hello Bob";
		CryptoPP::GCM<CryptoPP::AES>::Encryption e;
		e.SetKeyWithIV(Alice.key_,Alice.size_,Alice.iv_);
		std::string cipherText = Alice.encrypt(e,"AAD",message,tagSize);
		ASSERT_NE(message,cipherText);

		//Decryption from Bob
		CryptoPP::GCM<CryptoPP::AES>::Decryption d;
		d.SetKeyWithIV(Bob.key_,Bob.size_,Bob.iv_);
		std::string recoveredText;
		ASSERT_EQ(Bob.decrypt(d,tagSize,"AAD",cipherText,recoveredText),DECRYPT_OK);
	}
}
