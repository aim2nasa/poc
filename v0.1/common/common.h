#ifndef __COMMON_H__
#define __COMMON_H__

#define AES_MAX_KEY_SIZE	32	//32*8 = 256bit , Maximum key size that can be assigned
#define AES_KEY_SIZE		32	//AES Key size used in this project 

#define TAG_KEY_LABEL	"GroupTagKey"	//Label given for each Group Tag
#define SE_KEY_LABEL	"SeKey"		//Label giben for each SE

#define USE_SSL				//If we use SSL(ACE SSL) then USE_SSL must be defined Otherwise SSL disabled 

#ifdef _WIN32
#define MODEXPORT __declspec(dllexport)
#else
#define MODEXPORT 
#endif

#ifdef _WIN32
#define SPRINTF sprintf_s
#else
#define SPRINTF snprintf
#endif

#endif
