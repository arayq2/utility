#pragma once

#include <vector>
#include <algorithm>
#include <thread>
#include <utility>

namespace Utility
{
    template<typename Functor>
    class ThreadPool
    {
    public:
        using Workers = std::vector<std::thread>;
        
        explicit
        ThreadPool(Functor fn, size_t count = 0)
        : fn_(fn)
        , workers_()
        , stopped_(true)
        {
            if ( count > 0 ) { start( count ); }
        }
        
        ~ThreadPool() { stop(); }
        
        void start( size_t count )
        {
            if ( stopped_ )
            {
                fill( count );
                stopped_ = false;
            }
        }
        
        void stop()
        {
            if ( !stopped_ )
            {
                stopped_ = true;
                drain();
            }
        }
    
    private:
        Functor     fn_;
        Workers     workers_;
        bool        stopped_;
        
        void fill( size_t count )
        {
            workers_.reserve( count );
            std::generate_n( std::back_inserter(workers_), count, [this]()
            {
                return std::thread(fn_);
            } );
        }
        
        void drain()
        {
            for ( auto& worker : workers_ ) { worker.join(); }
            workers_.clear();
        }
    };
} // namespace Utility
