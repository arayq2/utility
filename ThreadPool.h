#pragma once

#include <vector>
#include <algorithm>
#include <thread>
#include <utility>
#include <pthread.h>

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
                fill_( count );
                stopped_ = false;
            }
        }

        void stop()
        {
            if ( !stopped_ )
            {
                stopped_ = true;
                drain_();
            }
        }

        void cancel( std::thread::id const& id = std::thread::id(), bool detach = false )
        {
            if ( stopped_ ) { return; } // nothing to do
            if ( id == std::thread::id() )
            {
                stopped_ = true;
                cancel_all_( detach );
            }
            else { cancel_( id, detach ); }
        }

    private:
        Functor     fn_;
        Workers     workers_;
        bool        stopped_;

        void fill_( size_t count )
        {
            std::generate_n( std::back_inserter(workers_), count, [this]()
            {
                return std::thread(fn_);
            } );
        }

        void drain_()
        {
            for ( auto& _worker : workers_ ) { _worker.join(); }
            workers_.clear();
        }

        void cancel_thread_( std::thread& worker, bool detach )
        {
            ::pthread_cancel( worker.native_handle() );
            if ( detach ) { worker.detach(); }
            else { worker.join(); }
        }

        void cancel_( std::thread::id id, bool detach )
        {
            auto    _lambda([id, detach]( std::thread& worker ) -> bool
            {
                if ( worker.get_id() != id ) { return false; }
                cancel_thread_( worker, detach );
                return true;
            });
            workers_.erase( std::remove_if( workers_.begin(), workers_.end(), _lambda ), workers_.end() );
        }

        void cancel_all_( bool detach )
        {
            for ( auto& _worker : workers_ ) { cancel_thread_( _worker, detach ); }
            workers_.clear();
        }
    };
} // namespace Utility

