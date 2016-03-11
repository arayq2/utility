#pragma once

#include <vector>
#include <string>
#include <exception>
#include <boost/algorithm/string.hpp>

namespace Utility
{
    class BoostSplitter
    {
    public:
        struct Exception : public std::exception {};
        using value_type = std::string;
        using iterator   = std::vector<std::string>::iterator;
        
        BoostSplitter(std::string const& input, char const* stops, size_t minTok = 0)
        {
            boost::split( items_, input, boost::is_any_of( stops ) );
            if ( items_.size() < minTok ) { throw Exception(); }
        }
        
        iterator begin() { return items_.begin(); }
        iterator end() { return items_.end(); }
        
        operator std::vector<std::string> const& () const { return items_; }
    
        std::string const& operator[] ( size_t index ) const { return items_.at( index ); }
    
    private:
        std::vector<std::string>    items_;
    };
} // namespace Utility
