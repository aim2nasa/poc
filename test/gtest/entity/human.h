#ifndef __HUMAN_H__
#define __HUMAN_H__

#include "keyStore.h"
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>

class Human : public KeyStore{
public:
	Human(){ memset(iv_,0,sizeof(iv_)); }
	~Human(){}

	template <typename T> 
	std::string transform(T e,std::string input){ 
		std::string output;
		CryptoPP::StringSource(input,true,
			new CryptoPP::StreamTransformationFilter(e,
				new CryptoPP::StringSink(output)
			)
		);	
		return output;
	}

	byte iv_[CryptoPP::AES::BLOCKSIZE];
};

#endif
