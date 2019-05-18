#include "timer.h"
#include "demo.h"

uint16 getTime()
{
    return *my_clock;
}

void sleep(uint8 secs)
{
    float timeDiff = 0.0f;
    float timeEnd = (float)secs * TIME_FREQ;
    uint16 startTime  = getTime();
    while(timeDiff < timeEnd)
    {
        timeDiff = getTime() - startTime;
    }
}
