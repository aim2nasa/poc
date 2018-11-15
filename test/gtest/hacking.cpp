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

	int tagSize = 16;
	std::string cipherText;
	//Secret message fron Alice to Bob
	{
		//Encryption from Alice
		std::string message = "Hello Bob";
		CryptoPP::GCM<CryptoPP::AES>::Encryption e;
		e.SetKeyWithIV(Alice.key_,Alice.size_,Alice.iv_);
		cipherText = Alice.encrypt(e,"AAD",message,tagSize);
		ASSERT_NE(message,cipherText);

		//Decryption from Bob
		CryptoPP::GCM<CryptoPP::AES>::Decryption d;
		d.SetKeyWithIV(Bob.key_,Bob.size_,Bob.iv_);
		std::string recoveredText;
		ASSERT_EQ(Bob.decrypt(d,tagSize,"AAD",cipherText,recoveredText),DECRYPT_OK);
	}

	//Eve, doesn't have the key Alice and Bob share
	{
		Node Eve;
		Eve.size_ = 32;
		Eve.key_ = new byte[Eve.size_];
		ASSERT_NE(memcmp(Eve.key_,Alice.key_,Eve.size_),0);
		ASSERT_NE(memcmp(Eve.iv_,Alice.iv_,CryptoPP::AES::BLOCKSIZE),0);

		//Eavesdrop Alice's encrypted message(i.e. cipherText)
		//Decryption has to end up fail because Eve doesn't have key,iv
		CryptoPP::GCM<CryptoPP::AES>::Decryption d;
		d.SetKeyWithIV(Eve.key_,Eve.size_,Eve.iv_);
		std::string recoveredText;
		ASSERT_EQ(Eve.decrypt(d,tagSize,"AAD",cipherText,recoveredText),ERROR_HASH_VERIFY_FAILED);
		ASSERT_EQ(recoveredText.size(),0);
	}
}

TEST(hackingTest, unauthorised_Publication) { 
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

	int tagSize = 16;
	std::string cipherText;
	//Secret message fron Alice to Bob
	{
		//Encryption from Alice
		std::string message = "Hello Bob";
		CryptoPP::GCM<CryptoPP::AES>::Encryption e;
		e.SetKeyWithIV(Alice.key_,Alice.size_,Alice.iv_);
		cipherText = Alice.encrypt(e,"AAD",message,tagSize);
		ASSERT_NE(message,cipherText);

		//Decryption from Bob
		CryptoPP::GCM<CryptoPP::AES>::Decryption d;
		d.SetKeyWithIV(Bob.key_,Bob.size_,Bob.iv_);
		std::string recoveredText;
		ASSERT_EQ(Bob.decrypt(d,tagSize,"AAD",cipherText,recoveredText),DECRYPT_OK);
	}

	//Trudy, doesn't have the key Alice and Bob share(No authorization)
	{
		Node Trudy;
		Trudy.size_ = 32;
		Trudy.key_ = new byte[Trudy.size_];
		ASSERT_NE(memcmp(Trudy.key_,Alice.key_,Trudy.size_),0);
		ASSERT_NE(memcmp(Trudy.iv_,Alice.iv_,CryptoPP::AES::BLOCKSIZE),0);

		//Trudy creates manipulated message to send to Bob as if it comes from Alice
		std::string trudyMessage = "Bob you idiot";
		CryptoPP::GCM<CryptoPP::AES>::Encryption e;
		e.SetKeyWithIV(Trudy.key_,Trudy.size_,Trudy.iv_);
		cipherText = Trudy.encrypt(e,"AAD",trudyMessage,tagSize);
		ASSERT_NE(trudyMessage,cipherText);

		//Bob receives message from Trudy
		//Through MAC validation, Bob can find out the message is from the one who has key or not. 
		CryptoPP::GCM<CryptoPP::AES>::Decryption d;
		d.SetKeyWithIV(Bob.key_,Bob.size_,Bob.iv_);
		std::string recoveredText;
		ASSERT_EQ(Bob.decrypt(d,tagSize,"AAD",cipherText,recoveredText),ERROR_HASH_VERIFY_FAILED);
		ASSERT_EQ(recoveredText.size(),0);
	}
}
