#ifndef __COMMON_H__
#define __COMMON_H__

#define AES_MAX_KEY_SIZE	32	// 32*8 = 256bit , 본 프로젝트에서 사용가능한 최대의 Key size
#define AES_KEY_SIZE		32	//본 프로젝트에서 사용할 AES의 Key size (128bit)

#define TAG_KEY_LABEL	"GroupTagKey"	//토큰에서 Tag키를 찾기 위해 사용하는 라벨
#define SE_KEY_LABEL	"SeKey"			//토큰에서 SE키를 찾기 위해 사용하는 라벨

#define USE_SSL							//SSL(ACE SSL)을 사용하는 경우에 정의

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT 
#endif

#endif