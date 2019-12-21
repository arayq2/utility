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
        using Promise = std::promise<ValueType>;
        using Map     = std::map<std::string, Promise>;
    public:
        using Future  = std::future<ValueType>;
        
        ~AsynchMatcher() = default;
        AsynchMatcher() = default;
        
        Future reserve( std::string const& key )
        {
            AUTOLOCK();
            auto    _itr(map_.insert( std::make_pair( key, Promise() ) ).first);
            return _itr->second.get_future();
        }

        // for void ValueType only
        template<typename Ret = typename std::enable_if<std::is_void<ValueType>::value, bool>::type>
        Ret fulfill( std::string const& key )
        {
            AUTOLOCK();
            auto    _itr(map_.find( key ));
            
            if ( _itr != map_.end() )
            {
                _itr->second.set_value();
                map_.erase( _itr );
                return true;
            }
            return false;
        }

        // for non-void ValueType only
        template<typename VT = ValueType>
        typename std::enable_if<!std::is_void<VT>::value, bool>::type
        fulfill( std::string const& key, VT&& value )
        {
            AUTOLOCK();
            auto    _itr(map_.find( key ));
            
            if ( _itr != map_.end() )
            {
                _itr->second.set_value( std::forward<ValueType>(value) );
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
