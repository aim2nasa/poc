#ifndef __NODE_H__
#define __NODE_H__

#include "keyStore.h"
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/ccm.h>

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

	template <typename T>
	std::string encrypt(T e,std::string aad,std::string input) {
		e.SpecifyDataLengths(aad.size(),input.size(),0);
		std::string output;
		CryptoPP::AuthenticatedEncryptionFilter ef(e,new CryptoPP::StringSink(output));

		ef.ChannelPut("AAD",(const byte*)aad.data(),aad.size());
		ef.ChannelMessageEnd("AAD");
		ef.ChannelPut("",(const byte*)input.data(),input.size());
		ef.ChannelMessageEnd("");
		return output;
	}

	byte iv_[CryptoPP::AES::BLOCKSIZE];
};

#endif
