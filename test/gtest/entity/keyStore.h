#ifndef __KEY_STORE_H__
#define __KEY_STORE_H__

#include <cstddef>
typedef unsigned char byte; 

class KeyStore{
public:
	KeyStore():size_(0),key_(0){}
	~KeyStore(){ delete [] key_; }

	size_t size_;
	byte *key_;
};

#endif
