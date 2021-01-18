#include "../Exception.h"
#include <stdio.h>

class Bar
{
 public:
  void test()
  {
    throw ham::Exception("oops");
  }
};

void foo()
{
  Bar b;
  b.test();
}

int main()
{
  try
  {
    foo();
  }
  catch (const ham::Exception& ex)
  {
    printf("reason: %s\n", ex.what());
    printf("stack trace: %s\n", ex.stackTrace());
  }
}
