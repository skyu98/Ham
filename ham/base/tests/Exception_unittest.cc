#include "../Exception.h"
#include <iostream>
#include <string>
using namespace ham;

class Foo
{
public:
    void Bar()
    {   
        std::cout << "throw Ex!!!"<< '\n';
        throw Exception("Oops!!!");
    }
};

void func()
{
    Foo f;
    f.Bar();
}

int main()
{
    try
    {
        func();
    }
    catch(const Exception& ex)
    {
        std::cout << "Catch Ex!!!"<< '\n';
        std::cout << "reasons: " << std::string(ex.what()) << std::endl;
        std::cout << "stack trace: " << std::string(ex.stackTrace()) << std::endl;
    }
    return 0;
}