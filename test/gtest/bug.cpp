#include <gtest/gtest.h>
#include <entity/keyStore.h>
#include <entity/node.h>

TEST(bugTest, bug_NO265) { 
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

#include <cryptopp/hex.h>

TEST(bugTest, bug_NO275) {
	Node Alice;
	Alice.size_ = 32;
	Alice.key_ = new byte[Alice.size_];
	memset(Alice.key_,0,Alice.size_);
	memset(Alice.iv_,0,CryptoPP::AES::BLOCKSIZE);

	std::string adata(16, (char)0x00);
    char buffer[]={ "Hello" };
    ASSERT_EQ(strlen(buffer),5);
	std::string message(buffer);

	int tagSize = 16;
	CryptoPP::GCM<CryptoPP::AES>::Encryption e;
	e.SetKeyWithIV(Alice.key_,Alice.size_,Alice.iv_);
	std::string cipherText = Alice.encrypt(e,adata,message,tagSize);
    ASSERT_EQ(cipherText.size(),tagSize+strlen(buffer));    //16+5

    buffer[1]=0;    //Hello -> H'Null'llo
    ASSERT_EQ(strlen(buffer),1);
    std::string message1(buffer);

	cipherText = Alice.encrypt(e,adata,message1,tagSize);
    ASSERT_EQ(cipherText.size(),tagSize+strlen(buffer));    //16+1

	cipherText = Alice.encrypt(e,adata,reinterpret_cast<const byte*>(buffer),5,tagSize);
    ASSERT_EQ(cipherText.size(),tagSize+5);    //16+5

	CryptoPP::GCM<CryptoPP::AES>::Decryption d;
	d.SetKeyWithIV(Alice.key_,Alice.size_,Alice.iv_);
	std::string recoveredText;
	ASSERT_EQ(Alice.decrypt(d,tagSize,adata,cipherText,recoveredText),DECRYPT_OK);
	ASSERT_EQ(memcmp(recoveredText.c_str(),buffer,5),0);

    std::string encoded;
    CryptoPP::StringSource(recoveredText,true,
        new CryptoPP::HexEncoder(
            new CryptoPP::StringSink(encoded)
        )
    );
    std::cout<<encoded<<std::endl;
}
