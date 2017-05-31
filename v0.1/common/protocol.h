#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "common.h"

#define PREFIX_SIZE 8
#define HEADER_SIZE	(PREFIX_SIZE+6)	//2ea : and SERIAL_NO_SIZE:4bytes
#define SERIAL_NO_SIZE	AES_KEY_SIZE
#define GROUP_NAME_SIZE	AES_KEY_SIZE

//prefix
#define PRF_SERIALNO	"SERIALNO"
#define PRF_REQ_STAT	"REQ_STAT"
#define PRF_REQ_KEYG	"REQ_KEYG"
#define PRF_ACK_STAT	"ACK_STAT"
#define PRF_ACK_KEYG	"ACK_KEYG"
#define PRF_NTF_TAGS	"NTF_TAGS"	//Notify메세지로 Tag키와 와 SE의 키가 전달됨을 알려줌

#define TAG_KEY_LABEL	"GroupTagKey"	//토큰에서 Tag키를 찾기 위해 사용하는 라벨
#define SE_KEY_LABEL	"SeKey"			//토큰에서 SE키를 찾기 위해 사용하는 라벨

#endif