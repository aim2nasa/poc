#include "CSe.h"
#include "ace\OS_NS_string.h"

CSe::CSe()
:h_(CK_INVALID_HANDLE), cid_(-1)
{
	ACE_OS::memset(serialNo, 0, SERIAL_NO_SIZE);
}