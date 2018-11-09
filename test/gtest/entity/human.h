#ifndef __HUMAN_H__
#define __HUMAN_H__

#include "keyStore.h"
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>

class Human : public KeyStore{
public:
	Human(){}
	~Human(){}

	std::string ecbEncrypt(std::string input){ 
		CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption e;
		e.SetKey(key_,size_);

		std::string output;
		CryptoPP::StringSource(input,true,
			new CryptoPP::StreamTransformationFilter(e,
				new CryptoPP::StringSink(output)
			)
		);	
		return output;
	}

	std::string ecbDecrypt(std::string input){
		CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption d;
		d.SetKey(key_,size_);

		std::string output;
		CryptoPP::StringSource(input,true,
			new CryptoPP::StreamTransformationFilter(d,
				new CryptoPP::StringSink(output)
			)
		);
		return output;
	}
};

#endif
