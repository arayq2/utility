
#include "Column.h"

//==[ EXAMPLE ]========================================================

    class Example
    : public Utility::ColumnAccess<Example>
    {
    public:
        Example(int i, double d, std::string const& s)
        : intVar_(i)
        , dblVar_(d)
        , strVar_(s)
        {}
        
    private:
        int             intVar_;
        double          dblVar_;
        std::string     strVar_;

        friend class Utility::ColumnAccess<Example>;
    };

    template<>
    Utility::ColumnAccess<Example>::FieldMap<int>
    Utility::ColumnAccess<Example>::intMap_ =
    {
        { "int", &Example::intVar_ }
    };

    template<>
    Utility::ColumnAccess<Example>::FieldMap<double>
    Utility::ColumnAccess<Example>::dblMap_ =
    {
        { "dbl", &Example::dblVar_ }
    };

    template<>
    Utility::ColumnAccess<Example>::FieldMap<std::string>
    Utility::ColumnAccess<Example>::strMap_ =
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
    for ( auto const& _i : Example::intColumn<>( &exampleVec, "int" ) ) { std::cerr << "value: " << _i << std::endl; }
    for ( auto const& _d : Example::dblColumn<>( &exampleVec, "dbl" ) ) { std::cerr << "value: " << _d << std::endl; }
    for ( auto const& _s : Example::strColumn<>( &exampleVec, "str" ) ) { std::cerr << "value: " << _s << std::endl; }
    std::copy( newVals.begin(), newVals.end(), Example::dblColumn<>( &exampleVec, "dbl" ).begin() );
    for ( auto const& _d : Example::dblColumn<>( &exampleVec, "dbl" ) ) { std::cerr << "value: " << _d << std::endl; }

    return 0;
}
