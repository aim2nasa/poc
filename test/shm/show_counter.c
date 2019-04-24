// show_counter.c   counter.c가 공유 메모리에 넣는
// 카운터 문자열을 화면에 계속 출력합니다.

#include <stdio.h>      // printf()
#include <unistd.h>     // sleep()
#include <sys/ipc.h>
#include <sys/shm.h>

#define  KEY_NUM     9527
#define  MEM_SIZE    1024

int main( void)
{
    int   shm_id;
    void *shm_addr;

    if ( -1 == ( shm_id = shmget( (key_t)KEY_NUM, MEM_SIZE, IPC_CREAT|0666)))
    {
        printf( "공유 메모리 생성 실패\n");
        return -1;
    }

    if ( ( void *)-1 == ( shm_addr = shmat( shm_id, ( void *)0, 0)))
    {
        printf( "공유 메모리 첨부 실패\n");
        return -1;
    }

    while( 1 )
    {
        printf( "%s\n", (char *)shm_addr);    // 공유 메모리를 화면에 출력
        sleep( 1);
    }
    return 0;
}