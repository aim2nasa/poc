#ifndef __SNDRCV_H__
#define __SNDRCV_H__

#include <ace/os_include/sys/os_types.h>
#include <ace/OS_NS_unistd.h>

template <typename SOCK> ssize_t rcv(SOCK &sock, void* buffer, size_t len)
{
	ssize_t size;
	while (true){
		if ((size = sock.recv_n(buffer, len)) < 0) {
			ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) %p\n"), ACE_TEXT("rcv error")));
			ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) %s\n"), ACE_TEXT("sleep...")));
			ACE_OS::sleep(1);
			continue;
		}
		break;
	}
	return size;
}

#endif