#include "CSe.h"
#include "ace/OS_NS_string.h"
#include "ace/Assert.h"

CSe::CSe()
:h_(CK_INVALID_HANDLE), cid_(-1)
{
	ACE_OS::memset(tagKey_, 0, AES_KEY_SIZE);
	ACE_OS::memset(seKey_, 0, AES_KEY_SIZE);
}

void CSe::tagKey(CK_BYTE_PTR tKey, CK_ULONG size)
{
	ACE_ASSERT(size == AES_KEY_SIZE);
	ACE_OS::memcpy(tagKey_, tKey, size);
}

CK_BYTE_PTR CSe::tagKey()
{
	return tagKey_;
}

void CSe::seKey(CK_BYTE_PTR seKey, CK_ULONG size)
{
	ACE_ASSERT(size == AES_KEY_SIZE);
	ACE_OS::memcpy(seKey_, seKey, size);
}

CK_BYTE_PTR CSe::seKey()
{
	return seKey_;
}