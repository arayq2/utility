#pragma once

#include "Lockable.h"
#include <future>
#include <map>
#include <utility>

namespace Utility
{

    template<typename ValueType>
    class AsynchMatcher
    : private Lockable
    {
    public:
        using Promise = std::promise<ValueType>;
        using Map     = std::map<std::string, Promise>;
        using Future  = std::future<ValueType>;
        
        AsynchMatcher()
        : map_()
        {}
        
        Future reserve( std::string const& key )
        {
            AUTOLOCK();
            auto    _itr(map_.insert( std::make_pair( key, Promise() ) ).first);
            return _itr->second.get_future();
        }
        
        bool fulfill( std::string const& key, ValueType&& value )
        {
            AUTOLOCK();
            auto    _itr(map_.find( key ));
            
            if ( _itr != map_.end() )
            {
                _itr->second.set_value( std::move(value) );
                map_.erase( _itr );
                return true;
            }
            return false;
        }
        
        void cancel( std::string const& key )
        {
            AUTOLOCK();
            map_.erase( key );
        }
        
        size_t pending() const
        {
            AUTOLOCK();
            return map_.size();
        }       
        
    private:
        Map     map_;
        
        AsynchMatcher(AsynchMatcher const&) = delete;
        AsynchMatcher& operator=( AsynchMatcher const& ) = delete;
    };
    
} // namespace Utility
