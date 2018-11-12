#ifndef __NODE_H__
#define __NODE_H__

#include "keyStore.h"
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/ccm.h>

#define DECRYPT_OK			 			 0	
#define FAIL_GET_LAST_RESULT 			-1	
#define ERROR_TAG_SIZE_NOT_MATCH 	-100
#define ERROR_WRONG_INPUT_TAG_SIZE	-101

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

	template <typename T>
	int decrypt(T d,int tagSize,std::string aad,std::string input,std::string& output) {
		std::string enc = input.substr(0,input.size()-tagSize);
		std::string tag = input.substr(input.size()-tagSize);

		if(input.size()!=(enc.size()+tag.size())) return ERROR_TAG_SIZE_NOT_MATCH;
		if(tag.size()!=tagSize) return ERROR_WRONG_INPUT_TAG_SIZE;

		d.SpecifyDataLengths(aad.size(),enc.size(),0);

		CryptoPP::AuthenticatedDecryptionFilter df(d,NULL,
			CryptoPP::AuthenticatedDecryptionFilter::THROW_EXCEPTION);

		df.ChannelPut("AAD",(const byte*)aad.data(),aad.size());
		df.ChannelMessageEnd("AAD");

		df.ChannelPut("",(const byte*)enc.data(),enc.size());
		df.ChannelPut("",(const byte*)tag.data(),tag.size());
		df.ChannelMessageEnd("");

		if(!df.GetLastResult()) return FAIL_GET_LAST_RESULT;

		df.SetRetrievalChannel("");
		output.resize((size_t)df.MaxRetrievable());
		df.Get( (byte*)output.data(),output.size());

		return DECRYPT_OK; 
	}

	byte iv_[CryptoPP::AES::BLOCKSIZE];
};

#endif
