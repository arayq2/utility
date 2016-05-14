#pragma once

#include "BlockingQueue.h"
#include "ThreadPool.h"
#include <iterator>

namespace Utility
{
    template<typename> struct MonitorMethods;

    template<>
    struct MonitorMethods<Event>
    {
        static void signal( Event& event ) { event.signal(); }
        static size_t post( Event& event, size_t count ) { return event.post( count ); }
        static size_t cancel( Event& event, size_t count ) { return event.cancel( count ); }
        static void wait( Event& event ) { event.wait(); }
        static void stop( Event& event ) { event.stop(); }
   };
    
    template<typename Handler, typename Monitor = Event>
    class WorkPile
    {
    public:
        using Item   = typename Handler::ItemType;
        using Queue  = BlockingQueue<Item>;
        using Worker = typename Queue::template Worker<Handler, Monitor>;
        using Pool   = ThreadPool<Worker>;
        
        explicit
        WorkPile(Handler&& handler, size_t poolSize = 1)
        : monitor_(0)
        , queue_() // autostarted
        , worker_(queue_, std::move(handler), monitor_)
        , pool_(worker_, poolSize) // autostarted
        {}
        
        ~WorkPile() { stop(); }
        
        bool put( Item&& item )
        {
            MonitorMethods<Monitor>::post( monitor_, 1 );
            if ( !queue_.put( std::move(item) ) )
            {
                MonitorMethods<Monitor>::cancel( monitor_, 1 );
                return false;
            }   
            return true;
        }
        bool put( Item& item )
        {
            MonitorMethods<Monitor>::post( monitor_, 1 );
            if ( !queue_.put( item ) )
            {
                MonitorMethods<Monitor>::cancel( monitor_, 1 );
                return false;
            }
            return true;
        }

        template<typename Iterator>
        bool put( Iterator begin, Iterator end )
        {
            MonitorMethods<Monitor>::post( monitor_, std::distance( begin, end ) );
            if ( !queue_.batch_put( begin, end ) )
            {
                MonitorMethods<Monitor>::cancel( monitor_, std::distance( begin, end ) );
                return false;
            }
            return true;
        }
        
        void stop()
		{
			if ( !stopped_ )
			{
				stopped_ = true;
				queue_.stop();
				pool_.stop();
			}
		}
        
        void resume( size_t poolSize = 1 )
        {
            if ( stopped_ )
			{
				queue_.start();
				pool_.start( poolSize );
				stopped_ = false;
			}
        }
        
        void wait() { MonitorMethods<Monitor>::wait( monitor_ ); }
        
        size_t pending() { return MonitorMethods<Monitor>::post( monitor_, 0 ); }
		
		void cancel() { MonitorMethods<Monitor>::stop( monitor_ ); }

    private:
        Monitor monitor_;
        Queue   queue_;
        Worker  worker_;
        Pool    pool_;
        bool    stopped_ { false };
    };
} // namespace Utility
