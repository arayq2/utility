/** ======================================================================+
 + Copyright @2015-2021 Arjun Ray
 + Released under MIT License: see https://mit-license.org
 +========================================================================*/
#pragma once

#ifndef UTILITY_BASICQUEUE_H
#define UTILITY_BASICQUEUE_H

#include <deque>
#include <utility>
#include <mutex>
#include <condition_variable>

namespace Utility
{
    template<typename Item>
    class BasicQueue
    {
        using Mutex     = std::mutex;
        using Guard     = std::lock_guard<Mutex>;
        using Lock      = std::unique_lock<Mutex>;
        using Condition = std::condition_variable;
        using Queue     = std::deque<Item>;

    public:
        ~BasicQueue() noexcept = default;

        BasicQueue(bool stopped = false)
        : stopped_(stopped)
        {}

        std::size_t size() const
        {
            Guard   _guard(mx_);
            return queue_.size();
        }

        void stop()
        {
            Guard   _guard(mx_);
            stopped_ = true;
            ready_.notify_all();
        }

        void start()
        {
            Guard   _guard(mx_);
            stopped_ = false;
            if ( !queue_.empty() ) { ready_.notify_all(); }
        }

        void clear()
        {
            // drain( []( Item& ) {} );
            Guard   _guard(mx_);
            queue_.clear();
        }

        template<typename Handler>
        void restart( Handler&& handler )
        {
            stop();
            handler();
            start();
        }

        bool put( Item&& item )
        {
            Guard   _guard(mx_);
            if ( stopped_ ) { return false; }
            queue_.push_back( std::move(item) );
            ready_.notify_one();
            return true;
        }

        bool pop( Item& item )
        {
            Lock    _lock(mx_);
            while ( !stopped_ && queue_.empty() )
            {
                ready_.wait( _lock );
            }
            if ( stopped_ ) { return false; }
            item = std::move( queue_.front() );
            queue_.pop_front();
            return true;
        }

        template<typename Peeker>
        bool peek( Peeker&& peeker ) const
        {
            Lock    _lock(mx_);
            if ( queue_.empty() ) { return false; }
            peeker( std::cref(queue_.front()) ); // dangerous!
            return true;
        }

        template<typename Peeker>
        bool wait_for( Peeker&& peeker )
        {
            Lock    _lock(mx_);
            while ( !stopped_ && queue_.empty() )
            {
                ready_.wait( _lock );
            }
            if ( stopped_ ) { return false; }
            peeker( std::cref(queue_.front()) ); // dangerous!
            return true;
        }

        //!> Canonical usage
        template<typename Handler, typename... Args>
        void pump( Handler&& handler, Args&&... args )
        {
            while ( true )
            {
                Item    _item;
                if ( !pop( _item ) ) { return; }
                handler( _item, std::forward<Args>(args)... );
            }
        }

        template<typename Handler, typename... Args>
        void drain( Handler&& handler, Args&&... args )
        {
            Lock    _lock(mx_);
            while ( !queue_.empty() )
            {
                Item    _item(std::move(queue_.front()));
                queue_.pop_front();
                _lock.unlock();
                handler( _item, std::forward<Args>(args)... );
                _lock.lock();
            }
        }

        /**
         * Common use cases
         */

        template<typename Action>
        class Worker
        {
        public:
            Worker(BasicQueue& queue, Action&& action)
            : qp_(&queue)
            , action_(std::move(action))
            {}

            template<typename... Args>
            void operator()( Args&& ... args )
            {
                qp_->pump( action_, std::forward<Args>(args)... );
            }

        private:
            BasicQueue*     qp_;
            Action          action_;
        };

        class Runner
        {
        public:
            Runner(BasicQueue& queue)
            : queue_(queue)
            {}
            
            void operator()()
            {
                queue_.pump( []( Item& item )
                {
                    item();
                } );
            }

        private:
            BasicQueue&     queue_;
        };

    private:
        Mutex mutable   mx_; // for guard in const methods
        Condition       ready_;
        Queue           queue_;
        bool            stopped_;
    };

} // namespace Utility

#endif // UTILITY_BASICQUEUE_H
