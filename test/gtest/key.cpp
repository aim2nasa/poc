#include <gtest/gtest.h>
#include <entity/keyStore.h>
#include <entity/node.h>
 
TEST(keyStoreTest, basic) { 
	KeyStore store;

	int value = 7;	//random value

	store.size_=32;
	store.key_ = new byte[store.size_];
	memset(store.key_,value,store.size_);

	for(int i=0;i<store.size_;i++)
		ASSERT_EQ(store.key_[i],value);
}

TEST(nodeTest, ECB) { 
	Node h;

	h.size_=32;
	h.key_ = new byte[h.size_];
	memset(h.key_,0,h.size_);

	std::string plainText = "AES ECB Test";
	CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption e;
	e.SetKey(h.key_,h.size_);
	std::string cipherText = h.transform(e,plainText);

	ASSERT_NE(plainText,cipherText);

	CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption d;
	d.SetKey(h.key_,h.size_);
	std::string recoveredText = h.transform(d,cipherText);
	ASSERT_EQ(plainText,recoveredText);
}

TEST(nodeTest, CBC) { 
	Node h;

	h.size_=32;
	h.key_ = new byte[h.size_];
	memset(h.key_,0,h.size_);

	std::string plainText = "AES CBC Test";
	CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption e;
	e.SetKeyWithIV(h.key_,h.size_,h.iv_);
	std::string cipherText = h.transform(e,plainText);

	ASSERT_NE(plainText,cipherText);

	CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption d;
	d.SetKeyWithIV(h.key_,h.size_,h.iv_);
	std::string recoveredText = h.transform(d,cipherText);
	ASSERT_EQ(plainText,recoveredText);
}

TEST(nodeTest, OFB) { 
	Node h;

	h.size_=32;
	h.key_ = new byte[h.size_];
	memset(h.key_,0,h.size_);

	std::string plainText = "AES OFB Test";
	CryptoPP::OFB_Mode<CryptoPP::AES>::Encryption e;
	e.SetKeyWithIV(h.key_,h.size_,h.iv_);
	std::string cipherText = h.transform(e,plainText);

	ASSERT_NE(plainText,cipherText);

	CryptoPP::OFB_Mode<CryptoPP::AES>::Decryption d;
	d.SetKeyWithIV(h.key_,h.size_,h.iv_);
	std::string recoveredText = h.transform(d,cipherText);
	ASSERT_EQ(plainText,recoveredText);
}

TEST(nodeTest, CFB) { 
	Node h;

	h.size_=32;
	h.key_ = new byte[h.size_];
	memset(h.key_,0,h.size_);

	std::string plainText = "AES CFB Test";
	CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption e;
	e.SetKeyWithIV(h.key_,h.size_,h.iv_);
	std::string cipherText = h.transform(e,plainText);

	ASSERT_NE(plainText,cipherText);

	CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption d;
	d.SetKeyWithIV(h.key_,h.size_,h.iv_);
	std::string recoveredText = h.transform(d,cipherText);
	ASSERT_EQ(plainText,recoveredText);
}
