#include <gtest/gtest.h>
#include <entity/keyStore.h>
#include <entity/node.h>

TEST(encodingTest, bug_NO265) { 
	Node Alice;
	Alice.size_ = 32;
	Alice.key_ = new byte[Alice.size_];
	memset(Alice.key_,0,Alice.size_);
	memset(Alice.iv_,0,CryptoPP::AES::BLOCKSIZE);

	std::string adata(16, (char)0x00);
	std::string message = "Hellocrazy";

	int tagSize = 16;
	CryptoPP::GCM<CryptoPP::AES>::Encryption e;
	e.SetKeyWithIV(Alice.key_,Alice.size_,Alice.iv_);
	std::string cipherText = Alice.encrypt(e,adata,message,tagSize);

	CryptoPP::GCM<CryptoPP::AES>::Decryption d;
	d.SetKeyWithIV(Alice.key_,Alice.size_,Alice.iv_);
	std::string recoveredText;
	ASSERT_EQ(Alice.decrypt(d,tagSize,adata,cipherText,recoveredText),DECRYPT_OK);
	ASSERT_EQ(message,recoveredText);

	message = "HelloCrazy";

	cipherText = Alice.encrypt(e,adata,message,tagSize);
	ASSERT_EQ(Alice.decrypt(d,tagSize,adata,cipherText,recoveredText),DECRYPT_OK);
	ASSERT_EQ(message,recoveredText);
}
