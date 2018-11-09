#ifndef __NODE_H__
#define __NODE_H__

#include "keyStore.h"
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>

class Node : public KeyStore{
public:
	Node(){ memset(iv_,0,sizeof(iv_)); }
	~Node(){}

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
