#pragma once

#include <map>
#include <string>

namespace Utility
{
    // StringIndex.
    // Tracks string keys by index
    //
    class StringIndex
    {
    public:
        using Pair = std::pair<size_t, bool>;
        using Map  = std::map<std::string, size_t>;
        
        StringIndex()
        : map_()
        {}
        
        Pair operator() ( std::string const& key, size_t index )
        {
            auto    _pr(map_.insert( std::make_pair(key, index) ));
            return _pr.second ? Pair(index, true) : Pair(_pr.first->second, false);
        }
        
    private:
        Map     map_;
    };

} // namespace Utility
