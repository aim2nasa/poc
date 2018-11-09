#include <gtest/gtest.h>
#include <entity/keyStore.h>
#include <entity/human.h>
 
TEST(keyStoreTest, basic) { 
	KeyStore store;

	int value = 7;	//random value

	store.size_=32;
	store.key_ = new byte[store.size_];
	memset(store.key_,value,store.size_);

	for(int i=0;i<store.size_;i++)
		ASSERT_EQ(store.key_[i],value);
}

TEST(humanTest, basic) { 
	Human h;
	ASSERT_EQ(0,0);
}
