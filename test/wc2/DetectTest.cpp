#include "gmock/gmock.h"
#include "src/Detect.h"

TEST(Detect, run)
{
    Detect d;
    d.start(NULL);
    d.join();
}