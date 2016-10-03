
#include "Column.h"

//==[ EXAMPLE ]========================================================

    using namespace Utility;

    class Example
    : private AccessMap<Example, int>
    , private AccessMap<Example, double>
    , private AccessMap<Example, std::string>
    {
    public:
        Example(int i, double d, std::string const& s)
        : intVar_(i)
        , dblVar_(d)
        , strVar_(s)
        {}
        
        using rows   = std::vector<Example>;
        
        template<typename T, bool NoCheck = false>
        using column = Column<T, Example, NoCheck>;
        
        template<bool NoCheck = false>
        static column<int, NoCheck> intColumn( rows* matrix, std::string const& name )
        { return AccessMap<Example, int>::column<NoCheck>( matrix, name ); }

        template<bool NoCheck = false>
        static column<double, NoCheck> dblColumn( rows* matrix, std::string const& name )
        { return AccessMap<Example, double>::column<NoCheck>( matrix, name ); }

        template<bool NoCheck = false>
        static column<std::string, NoCheck> strColumn( rows* matrix, std::string const& name )
        { return AccessMap<Example, std::string>::column<NoCheck>( matrix, name ); }

    private:
        int             intVar_;
        double          dblVar_;
        std::string     strVar_;

        friend class AccessMap<Example, int>;
        friend class AccessMap<Example, double>;
        friend class AccessMap<Example, std::string>;
    };

    template<>
    AccessMap<Example, int>::FieldMap
    AccessMap<Example, int>::map_ =
    {
        { "int", &Example::intVar_ }
    };

    template<>
    AccessMap<Example, double>::FieldMap
    AccessMap<Example, double>::map_ =
    {
        { "dbl", &Example::dblVar_ }
    };

    template<>
    AccessMap<Example, std::string>::FieldMap
    AccessMap<Example, std::string>::map_ =
    {
        { "str", &Example::strVar_ }
    };

    
    std::vector<Example> exampleVec =
    {
        { 1, 1.1, "one"   },
        { 2, 2.2, "two"   },
        { 3, 3.3, "three" },
    };

    std::vector<double> newVals = { 4.4, 5.5, 6.6 };
    
#include <iostream>
#include <algorithm>

int main()
{
    for ( auto const& _i : Example::intColumn<>( &exampleVec, "int" ) ) { std::cerr << "int value: " << _i << std::endl; }
    for ( auto const& _d : Example::dblColumn<>( &exampleVec, "dbl" ) ) { std::cerr << "dbl value: " << _d << std::endl; }
    for ( auto const& _s : Example::strColumn<>( &exampleVec, "str" ) ) { std::cerr << "str value: " << _s << std::endl; }
    std::copy( newVals.begin(), newVals.end(), Example::dblColumn<>( &exampleVec, "dbl" ).begin() );
    for ( auto const& _d : Example::dblColumn<>( &exampleVec, "dbl" ) ) { std::cerr << "dbl value: " << _d << std::endl; }

    return 0;
}
