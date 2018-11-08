#ifndef __KEY_STORE_H__
#define __KEY_STORE_H__

#include <cryptopp/config.h>

class KeyStore{
public:
	KeyStore():size_(0),key_(0){}

	size_t size_;
	byte *key_;
};

#endif
