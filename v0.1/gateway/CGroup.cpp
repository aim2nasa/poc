#include "CGroup.h"
#include "ace/OS_NS_string.h"
#include "ace/Assert.h"

CGroup::CGroup()
:hGroup_(CK_INVALID_HANDLE), hTag_(CK_INVALID_HANDLE)
{
	ACE_OS::memset(tagKey_, 0, AES_KEY_SIZE);
}

CGroup::~CGroup()
{

}

void CGroup::tagKey(CK_BYTE_PTR tKey, CK_ULONG size)
{
	ACE_ASSERT(size == AES_KEY_SIZE);
	ACE_OS::memcpy(tagKey_, tKey, size);
}

CK_BYTE_PTR CGroup::tagKey()
{
	return tagKey_;
}