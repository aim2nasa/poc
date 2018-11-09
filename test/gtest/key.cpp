#include <gtest/gtest.h>
#include <entity/keyStore.h>
#include <entity/human.h>
 
TEST(keyStoreTest, basic) { 
	KeyStore store;

	int value = 7;	//random value

	store.size_=32;
	store.key_ = new byte[store.size_];
	memset(store.key_,value,store.size_);

	for(int i=0;i<store.size_;i++)
		ASSERT_EQ(store.key_[i],value);
}

TEST(humanTest, ECB) { 
	Human h;

	h.size_=32;
	h.key_ = new byte[h.size_];
	memset(h.key_,0,h.size_);

	std::string plainText = "AES ECB Test";
	CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption e;
	std::string cipherText = h.transform(e,plainText);

	ASSERT_NE(plainText,cipherText);

	CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption d;
	std::string recoveredText = h.transform(d,cipherText);
	ASSERT_EQ(plainText,recoveredText);
}
