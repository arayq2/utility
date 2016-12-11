
#include "SequenceJoin.h"

#include <iostream>

    template<typename T>
    class Bracket
    {
    public:
        Bracket(T const& t)
        : t_(t)
        {}

        friend
        std::ostream& operator<< ( std::ostream& os, Bracket<T> const& b )
        {
            os << "[" << b.t_ << "]";
            return os;
        }

    private:
        T const&    t_;
    };

int
main( int ac, char* av[] )
{
    std::cout << Utility::SequenceJoin<char**>( av + 1, av + ac ) << std::endl;
    std::cout << Utility::SequenceJoin<char**>( av + 1, av + ac, ":" ) << std::endl;
    std::cout << Utility::SequenceJoin<char**, Bracket<char*> >( av + 1, av + ac, "" ) << std::endl;
    return 0;
}
