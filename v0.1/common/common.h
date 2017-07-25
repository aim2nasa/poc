#ifndef __COMMON_H__
#define __COMMON_H__

#define AES_MAX_KEY_SIZE	32	// 32*8 = 256bit , �� ������Ʈ���� ��밡���� �ִ��� Key size
#define AES_KEY_SIZE		32	//�� ������Ʈ���� ����� AES�� Key size (128bit)

#define TAG_KEY_LABEL	"GroupTagKey"	//��ū���� TagŰ�� ã�� ���� ����ϴ� ��
#define SE_KEY_LABEL	"SeKey"			//��ū���� SEŰ�� ã�� ���� ����ϴ� ��

#define USE_SSL							//SSL(ACE SSL)�� ����ϴ� ��쿡 ����

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT 
#endif

#endif