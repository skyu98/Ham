#include "../Timestamp.h"
#include <vector>
#include <string>

using namespace ham;

void passByRef(const Timestamp& T)
{
    printf("%s\n", T.toString().c_str());
}

void passByVal(Timestamp T)
{
    printf("%s\n", T.toString().c_str());
}

void benchmark()
{
    const int kNumber = 1000 * 1000;
    std::vector<Timestamp> stamps;
    stamps.reserve(kNumber);

    for(int i = 0;i < kNumber;++i)
    {
        stamps.emplace_back(Timestamp::now());
    }
    printf("begin at %s\n", stamps.front().toString().c_str());
    printf("end at %s\n", stamps.back().toString().c_str());
    printf("time difference is %f\n", timeDifference(stamps.back(), stamps.front()));

    std::vector<int64_t> gaps(100);
    int64_t cur = stamps.front().microsecondsFromEpoch(), next; 
    for(int i = 1;i < kNumber;++i)
    {
        next = stamps[i].microsecondsFromEpoch();
        int64_t gap = next - cur;
        cur = next;

        if(gap < 0)
        {
            printf("reverse!!!\n");
        }
        else if(gap <= 100)
        {
            ++gaps[gap - 1];
        }
        else
        {
            printf("Big gap!!!\n");
        }
    }

    for(int i = 0;i < 100;++i)
    {
        printf("%d, %lld\n", i, gaps[i]);
    }
}

int main()
{
    Timestamp now = Timestamp::now();
    printf("now is %s\n", now.toString().c_str());
    passByRef(now);
    passByVal(now);
    benchmark();
    return 0;
}