#include <iostream>

static const double const_real = 3.14159;

static const char* const_str = "Hello";
static const bool const_bool = false;

class MyClass
{
public:
    static const int const_int = 13;

};

int main()
{
    static const int const_int = 42;

    std::cout << const_int  << ", "
             << const_real  << ", "
             << const_str   << ", "
             << const_bool  << std::endl;

    std::cout << MyClass::const_int << std::endl;

    return 0;
}

