
#include "Cache.h"
#include <string>
#include <sstream>
#include <iostream>

    class ExpensiveFunction
    {
    public:
        ExpensiveFunction(size_t& count)
        : count_(count)
        {}
        
        std::string operator() ( std::string const& input )
        {
            ++count_;
            return input + "_val";
        }
        
    private:
        size_t&  count_;
    };


static size_t count = 0;
    
std::string expfn( std::string const& input )
{
    ++count;
    return input + "_val";
}

void print_key( std::string const& key ) 
{
    std::cerr << "[" << key << "]";
}
  
void test_tree()
{
    using TreeCache = Utility::TreeMapCache<std::string, std::string>;
    TreeCache           _tcache(5, expfn);

    std::cerr << "\nTesting TreeCache...\n";

    std::cerr << "Caching one: "   << _tcache( "one" ) << "\n";
    std::cerr << "Caching two: "   << _tcache( "two" ) << "\n";
    std::cerr << "Caching three: " << _tcache( "three" ) << "\n";
    std::cerr << "Caching four: "  << _tcache( "four" ) << "\n";
    std::cerr << "Caching five: "  << _tcache( "five" ) << "\n";
    
    std::cerr << "Getting one: "   << _tcache( "one" ) << "\n";
    std::cerr << "Getting four: "  << _tcache( "four" ) << "\n";
    std::cerr << "Getting four: "  << _tcache( "four" ) << "\n";
    std::cerr << "Caching six: "   << _tcache( "six" ) << "\n";
    std::cerr << "Count: " << count << "\n";
    _tcache.apply( print_key );
    std::cerr << "\n";
}
 
void test_hash()
{
    using HashCache = Utility::HashMapCache<std::string, std::string>;
    
    size_t              _count{0};
    ExpensiveFunction   _ef(_count);
    HashCache           _hcache(5, _ef);

    std::cerr << "\nTesting HashCache...\n";

    std::cerr << "Caching one: "   << _hcache( "one" ) << "\n";
    std::cerr << "Caching two: "   << _hcache( "two" ) << "\n";
    std::cerr << "Caching three: " << _hcache( "three" ) << "\n";
    std::cerr << "Caching four: "  << _hcache( "four" ) << "\n";
    std::cerr << "Caching five: "  << _hcache( "five" ) << "\n";
    
    std::cerr << "Getting one: "   << _hcache( "one" ) << "\n";
    std::cerr << "Getting four: "  << _hcache( "four" ) << "\n";
    std::cerr << "Getting four: "  << _hcache( "four" ) << "\n";
    std::cerr << "Caching six: "   << _hcache( "six" ) << "\n";
    std::cerr << "Count: " << _count << "\n";
    for ( auto& _key : _hcache ) { print_key( _key ); }
    std::cerr << "\n";
}
     
int main()
{
    test_tree();
    test_hash();
    return 0;
}
