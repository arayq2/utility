#pragma once

#include "Lockable.h"
#include <deque>
#include <utility>

namespace Utility
{
    template<typename> struct MonitorMethods;
        
    template<typename Item>
    class BlockingQueue
    : private Lockable
    {
    public:
        BlockingQueue(bool stopped = false)
        : queue_()
        , ready_()
        , stopped_(stopped) {}
        
        ~BlockingQueue() {}
        
        void stop()
        {
            AUTOLOCK();
            stopped_ = true;
            ready_.broadcast();
        }
        
        void start()
        {
            AUTOLOCK();
            stopped_ = false;
            if ( !queue_.empty() ) { ready_.broadcast(); }
        }
        
        template<typename Action>
        void restart( Action&& action )
        {
            stop();
            action();
            start();
        }
        
        bool put( Item&& item )
        {
            AUTOLOCK();
            if ( stopped_) { return false; }
            queue_.push_back( std::move(item) );
            ready_.signal();
            return true;
        }

        bool put( Item& item )
        {
            AUTOLOCK();
            if ( stopped_) { return false; }
            queue_.push_back( item );
            ready_.signal();
            return true;
        }

        template<typename Iterator>
        bool batch_put( Iterator begin, Iterator end )
        {
            AUTOLOCK();
            if ( stopped_) { return false; }
            queue_.insert( queue_.end(), begin, end );
            ready_.broadcast();
            return true;            
        }
        
        bool pop( Item& item )
        {
            THISLOCK(_lock);
            while ( !stopped_ and queue_.empty() )
            {
                ready_.wait( _lock );
            }
            if ( stopped_ ) { return false; }
            item = std::move(queue_.front());
            queue_.pop_front();
            return true;
        }
        
        template<typename Handler>
        void pump( Handler&& handler )
        {
            // while ( pop( _item ) ) { handler( _item ); }
            // would delay ~Item() invocation.
            while ( true )
            {
                Item    _item;
                if ( pop( _item ) ) { handler( _item ); }
                else                { break; }
            }
        }
        
        /**
         * Helper classes for common use cases
         */
        template<typename Action, typename Monitor = void>
        class Worker
        {
        public:
            Worker(BlockingQueue& queue, Action&& action, Monitor& monitor)
            : queue_(queue)
            , action_(std::move(action))
            , monitor_(monitor)
            {}
            
            void operator() ()
            {
                queue_.pump( [this]( Item& item )
                {
                   action_( std::move(item) );
                   MonitorMethods<Monitor>::signal( monitor_ );
                } );
            }

        private:
            BlockingQueue&  queue_;
            Action          action_;
            Monitor&        monitor_;
        };
    
        template<typename Action>
        class Worker<Action, void>
        {
        public:
            Worker(BlockingQueue& queue, Action&& action)
            : queue_(queue)
            , action_(std::move(action))
            {}
            
            void operator() ()
            {
                queue_.pump( [this]( Item& item )
                {
                   action_( std::move(item) );
                } );
            }

        private:
            BlockingQueue&  queue_;
            Action          action_;
        };
    
        class Runner
        {
        public:
            Runner(BlockingQueue& queue)
            : queue_(queue)
            {}
            
            void operator() ()
            {
                queue_.pump( []( Item& item )
                {
                    item();
                } );
            }
            
        private:
            BlockingQueue&  queue_;
        };

    private:
        std::deque<Item>    queue_;
        Condition           ready_;
        bool                stopped_;
    };
} // namespace Utility
