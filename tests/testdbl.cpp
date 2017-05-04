
#include "ValueParser.h"
#include <iostream>

using namespace Utility;

int main()
{
    std::string     _line;
    
    while ( std::getline( std::cin, _line ) )
    {
        if ( _line.length() == 0 ) { break; }
        std::cout << "[" << _line << "]:" << std::endl;
        for ( auto const& _item : DoubleParser(_line) )
        {
            std::cout << "[" << _item << "]" << std::endl;
        }
    }
    return 0;
}