#pragma once

#include <vector>
#include <algorithm>

namespace Utility
{
    template<bool HEAP_SMALLEST = false>
    class Indexer
    {
        struct Pair
        {
            Pair(size_t index = 0)
            : index_(index)
            , total_(0)
            {}
            
            struct Comparator
            {
                bool operator() ( Pair const& lhs, Pair const& rhs )
                {
                    return HEAP_SMALLEST ? rhs.total_ < lhs.total_ : lhs.total_ < rhs.total_;
                }
            };
            
            size_t  index_;
            size_t  total_;
        };
   
        using Comparator = typename Indexer<HEAP_SMALLEST>::Pair::Comparator;

    public:
        Indexer(size_t size)
        : array_(size)
        {
            size_t  _ctr(0);
            for ( auto& _item : array_ ) { _item.index_ = _ctr++; }
            std::make_heap( array_.begin(), array_.end(), Comparator() );
        }
        
        size_t operator() ( size_t size )
        {
            // extract smallest total
            std::pop_heap( array_.begin(), array_.end(), Comparator() );
            // save index
            size_t      _index(array_.back().index_);
            // update and reheap
            array_.back().total_ += size;
            std::push_heap( array_.begin(), array_.end(), Comparator() );
            
            return _index;
        }
        
        template<typename Action>
        void for_each( Action&& action ) const
        {
            for ( auto const& _item : array_ ) { action( _item.index_, _item.total_ ); }
        }
        
    private:
        std::vector<Pair>  array_;
    };
    
    template<typename ValueType>
    class Distributor
    {
        using List  = std::vector<ValueType>;
        using Lists = std::vector<List>;
        
    public:
        Distributor(size_t size)
        : indexer_(size)
        , lists_(size)
        {}
        
        void assign( ValueType const& value, size_t size )
        {
            lists_[indexer_( size )].push_back( value );
        }

        template<typename Action>
        void for_each_list( Action&& action ) const
        {
            for ( auto const& _list : lists_ ) { action( _list ); }
        }
        
        template<typename Action>
        void for_each_index( Action&& action ) const
        {
            indexer_.for_each( [&action, this]( size_t index, size_t total )
            {
                action( total, static_cast<List const&>(lists_[index]) );
            } );
        }
        
    private:
        Indexer<true>   indexer_;
        Lists           lists_;
    };

} // namespace Utility
