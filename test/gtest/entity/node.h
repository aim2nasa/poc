#ifndef __NODE_H__
#define __NODE_H__

#include "keyStore.h"
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/ccm.h>
#include <cryptopp/gcm.h>
#include <stdexcept>

#define DECRYPT_OK			 			 0	
#define FAIL_GET_LAST_RESULT 			-1	
#define ERROR_TAG_SIZE_NOT_MATCH 	-100
#define ERROR_WRONG_INPUT_TAG_SIZE	-101
#define ERROR_HASH_VERIFY_FAILED 	-102
#define ERROR_OUT_OF_RANGE       	-103
#define ERROR_UNCAUGHT_EXCEPTION  	-1000

class Node : public KeyStore{
public:
	Node(){ memset(iv_,0,sizeof(iv_)); }
	~Node(){}

	class EFilter{
	public:
		EFilter():ef_(NULL){}
		~EFilter(){ delete ef_; }
		CryptoPP::AuthenticatedEncryptionFilter *ef_;
	};

	class DFilter{
	public:
		DFilter():df_(NULL){}
		~DFilter(){ delete df_; }
		CryptoPP::AuthenticatedDecryptionFilter *df_;
	};

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
	std::string encrypt(T e,std::string aad,std::string input,int tagSize=0) {
		e.SpecifyDataLengths(aad.size(),input.size(),0);
		std::string output;

		EFilter f;
		if(tagSize<=0)
			f.ef_ = new CryptoPP::AuthenticatedEncryptionFilter(e,new CryptoPP::StringSink(output));
		else
			f.ef_ = new CryptoPP::AuthenticatedEncryptionFilter(e,new CryptoPP::StringSink(output),false,tagSize);

		f.ef_->ChannelPut("AAD",(const byte*)aad.data(),aad.size());
		f.ef_->ChannelMessageEnd("AAD");
		f.ef_->ChannelPut("",(const byte*)input.data(),input.size());
		f.ef_->ChannelMessageEnd("");
		return output;
	}

	template <typename T>
	int decrypt(T d,int tagSize,std::string aad,std::string input,std::string& output) {
		try{
			std::string enc = input.substr(0,input.size()-tagSize);
			std::string tag = input.substr(input.size()-tagSize);

			if(input.size()!=(enc.size()+tag.size())) return ERROR_TAG_SIZE_NOT_MATCH;
			if(tag.size()!=tagSize) return ERROR_WRONG_INPUT_TAG_SIZE;

			d.SpecifyDataLengths(aad.size(),enc.size(),0);

			DFilter f;
			if(tagSize<=0)
				f.df_ = new CryptoPP::AuthenticatedDecryptionFilter(d,NULL,
					CryptoPP::AuthenticatedDecryptionFilter::THROW_EXCEPTION);
			else
				f.df_ = new CryptoPP::AuthenticatedDecryptionFilter(d,NULL,
					CryptoPP::AuthenticatedDecryptionFilter::THROW_EXCEPTION,tagSize);

			f.df_->ChannelPut("AAD",(const byte*)aad.data(),aad.size());
			f.df_->ChannelMessageEnd("AAD");

			f.df_->ChannelPut("",(const byte*)enc.data(),enc.size());
			f.df_->ChannelPut("",(const byte*)tag.data(),tag.size());
			f.df_->ChannelMessageEnd("");

			if(!f.df_->GetLastResult()) return FAIL_GET_LAST_RESULT;

			f.df_->SetRetrievalChannel("");
			output.resize((size_t)f.df_->MaxRetrievable());
			f.df_->Get( (byte*)output.data(),output.size());
		}catch(CryptoPP::HashVerificationFilter::HashVerificationFailed& e){
			return ERROR_HASH_VERIFY_FAILED;
		}catch(const std::out_of_range& oor){
			return ERROR_OUT_OF_RANGE;
		}catch(...){
			return ERROR_UNCAUGHT_EXCEPTION;
		}
		return DECRYPT_OK; 
	}

	static std::string errToStr(int errCode){
		switch(errCode){
		case DECRYPT_OK:
			return "Decrypt ok";
			break;
		case FAIL_GET_LAST_RESULT:
			return "Fail get last result";
			break;
		case ERROR_TAG_SIZE_NOT_MATCH:
			return "Error tag size not match";
			break;
		case ERROR_WRONG_INPUT_TAG_SIZE:
			return "Error wrong input tag size";
			break;
		case ERROR_HASH_VERIFY_FAILED:
			return "Error hash verify failed";
			break;
		case ERROR_OUT_OF_RANGE:
			return "Error out of range";
			break;
		case ERROR_UNCAUGHT_EXCEPTION:
			return "Error uncaught exception";
			break;
		default:
			return "Undefined error code";
		}
	}

	byte iv_[CryptoPP::AES::BLOCKSIZE];
};

#endif
