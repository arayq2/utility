
#include "Distributor.h"
#include <iostream>

using Distributor = Utility::Distributor<size_t>;

void output_lists( std::ostream& os, Distributor const& dist )
{
    dist.for_each_index( [&os]( size_t total, std::vector<size_t> const& list ) -> void
    {
        os << "[" << total << "]";
        for ( auto& _item : list ) { os << "," << _item; }
        os << std::endl;
    } );
}

int main()
{
    Distributor     _dist(100);
    size_t          _ctr(0);
    size_t          _size;
    
    while ( std::cin >> _size ) { _dist( _ctr++, _size ); }
    output_lists( std::cout, _dist );
    return 0;
}
