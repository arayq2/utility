#pragma once

#include <list>
#include <map>
#include <unordered_map>
#include <functional>

namespace Utility
{
    // Cache.h
    // LRU eviction based cache.
    // Note value type semantics (use smart pointers if needed)
    //
    template<typename Key, typename Value, template<typename...> class MapType>
    class Cache
    {
    public:
        using key_type   = Key;
        using value_type = Value;
        using Tracker    = std::list<key_type>; // stores keys in LRU order
        using iterator   = typename Tracker::iterator;
        using Pair       = std::pair<value_type, iterator>;
        using Storage    = MapType<key_type, Pair>;
        using Function   = std::function<value_type( key_type const& )>;
        
        Cache(std::size_t capacity, Function function)
        : capacity_(capacity)
        , function_(function)
        , tracker_()
        , storage_()
        {}
        
        // mimics interface of function_
        value_type operator() ( key_type const& key )
        {
            auto    _it(storage_.find( key ));
            
            if ( _it != storage_.end() )
            {   // cache hit: move key to tail
                tracker_.splice( tracker_.end(), tracker_, (*_it).second.second ); 
                return (*_it).second.first;
            }
            // cache miss: get new value first (for exception safety)
            value_type  _nv(function_( key ));
            // not completely safe!
            storage_[key] = std::make_pair(_nv, tracker_.insert( tracker_.end(), key ));
            // evict LRU entry if needed
            check_cap_();
            return _nv;
        }
        
        void remove( key_type const& key )
        {
            auto    _it(storage_.find( key ));
            
            if ( _it != storage_.end() )
            {   
                tracker_.erase( (*_it).second.second ); 
                storage_.erase( _it );
            }            
        }
        
        void clear()  // ouch
        {
            storage_.clear();
            tracker_.clear();
        }

        size_t capacity( size_t new_cap = 0 )
        {
            size_t  _cap(capacity_);
            if ( new_cap > 0 )
            {
                capacity_ = new_cap;
            }
            if ( capacity_ < _cap )
            {
                check_cap_();
            }
            return _cap;
        }

        // for iterating over keys
        iterator begin() { return tracker_.begin(); }
        iterator end()   { return tracker_.end();   }
        
        template<typename Action>
        void foreach_key( Action&& action ) const
        {
            for ( auto const& key : tracker_ ) { action( key ); }
        }      

    private:
        std::size_t capacity_;
        Function    function_;
        Tracker     tracker_;
        Storage     storage_;
        
        void check_cap_()
        {
            while ( storage_.size() > capacity_ )
            {
                storage_.erase( storage_.find( tracker_.front() ) );
                tracker_.pop_front();
            }
        }
    };

    template<typename Key, typename Value>
    using TreeMapCache = Cache<Key, Value, std::map>;
    
    template<typename Key, typename Value>
    using HashMapCache = Cache<Key, Value, std::unordered_map>;

} // namespace Utility
