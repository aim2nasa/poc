#include <gtest/gtest.h>
#include <entity/keyStore.h>
 
TEST(keyStoreTest, basic) { 
	KeyStore store;

	int value = 7;	//random value

	store.size_=32;
	store.key_ = new byte[store.size_];
	memset(store.key_,value,store.size_);

	for(int i=0;i<store.size_;i++)
		ASSERT_EQ(store.key_[i],value);
}
