
#include "Column.h"

#include <string>

//==[ EXAMPLE ]========================================================

    class Example
    {
    public:
        Example(int i, double d, std::string const& s)
        : intVar_(i)
        , dblVar_(d)
        , strVar_(s)
        {}
        
        static int Example::*         getIntFld() { return &Example::intVar_; }
        static double Example::*      getDblFld() { return &Example::dblVar_; }
        static std::string Example::* getStrFld() { return &Example::strVar_; }
        
    private:
        int             intVar_;
        double          dblVar_;
        std::string     strVar_;
    };
    
    using Utility::Column;
    
    Column<int, Example> getIntColumn( std::vector<Example>* matrix )
    {
        return Column<int, Example>(matrix, Example::getIntFld());
    }

    Column<double, Example> getDblColumn( std::vector<Example>* matrix )
    {
        return Column<double, Example>(matrix, Example::getDblFld());
    }

    Column<std::string, Example> getStrColumn( std::vector<Example>* matrix )
    {
        return Column<std::string, Example>(matrix, Example::getStrFld());
    }

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
    for ( auto const& _i : getIntColumn( &exampleVec ) ) { std::cerr << "value: " << _i << std::endl; }
    for ( auto const& _d : getDblColumn( &exampleVec ) ) { std::cerr << "value: " << _d << std::endl; }
    for ( auto const& _s : getStrColumn( &exampleVec ) ) { std::cerr << "value: " << _s << std::endl; }
    // wait, it gets better!
    std::copy( newVals.begin(), newVals.end(), getDblColumn( &exampleVec ).begin() );
    for ( auto const& _d : getDblColumn( &exampleVec ) ) { std::cerr << "value: " << _d << std::endl; }

    return 0;
}