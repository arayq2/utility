
#include "Distributor.h"
#include <iostream>

using Distributor = Utility::Distributor<size_t>;

void output_lists( std::ostream& os, Distributor const& dist )
{
    dist.for_each( [&os]( size_t total, std::vector<size_t> const& list ) -> void
    {
        os << "[" << total << "]";
        for ( auto& _index : list ) { os << "," << _index; }
        os << std::endl;
    } );
}

int main()
{
    Distributor     _dist(100);
    size_t          _index(0);
    size_t          _size;
    
    while ( std::cin >> _size ) { _dist.assign( _index++, _size ); }
    output_lists( std::cout, _dist );
    return 0;
}
