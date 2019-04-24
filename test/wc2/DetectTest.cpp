#include "gmock/gmock.h"
#include "src/Detect.h"

TEST(Detect, run)
{
    Detect d;
    d.start(&d);
    d.stop();
    d.join();
}

TEST(Detect, messages)
{
    Detect d;
    d.start(&d);

    char msg[128];
    for(int i=0;i<5;i++) {
        sprintf(msg,"Message #%d",i);
        d.recv((void*)msg,strlen(msg));
    }

    d.stop();
    d.join();
}
