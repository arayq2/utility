
#include "StringJoin.h"

#include <iostream>

int
main( int ac, char* av[] )
{
    std::cout << Utility::StringJoin<char**>( av + 1, av + ac, ":") << std::endl;
    return 0;
}