#pragma once

#include "SpinLock.h"
#include <future>
#include <deque>
#include <thread>
#include <utility>
#include <stdexcept>

namespace Utility
{
    /**
     * WaitQueue.
     * A re-entrant mutex with enqueued clients (waiters).
     * The promise at the front is fulfilled for acquire(), and 
     * remains on the queue as a placeholder until release(), at 
     * which point it is dismissed and the successor is fulfilled.
     */
    class WaitQueue
    {
    public:
        WaitQueue() = default;
        // promises still on the queue will set_exception() on their futures
        ~WaitQueue() noexcept = default;
    
        void lock() { append( std::this_thread::get_id() ).get(); }
        
        void unlock() { remove( std::this_thread::get_id() ); }

    private:
        using ThdId   = std::thread::id;
        using Promise = std::promise<void>; // producer of ready signal
        using Future  = std::future<void>;  // consumer of ready signal
        using Pair    = std::pair<ThdId, Promise>;
        using Queue   = std::deque<Pair>; // acquisition order
        using Flag    = std::atomic_flag; 
        
        Queue       queue_;
        ThdId       curr_; // current holder of "lock"
        Promise     rpt_;   // non-queue placeholder for re-entrancy
        unsigned    count_; // re-entrancy count
        Flag        flag_{ATOMIC_FLAG_INIT}; // mutex
        
        // enable front of queue and mark its id
        void fulfill( Pair& pr )
        {
            curr_  = pr.first;
            count_ = 1;
            pr.second.set_value();
        }
        
        // re-entrancy is not cheap!
        Future re_enter()
        {
            rpt_ = Promise();
            ++count_;
            rpt_.set_value();
            return rpt_.get_future();      
        }
        
        // new promise, fulfilled immediately if the only one
        Future append( ThdId id )
        {
            SpinLock    _lock(flag_);
            
            if ( curr_ == id ) { return re_enter(); }
            
            queue_.emplace_back( std::make_pair( id, Promise() ) );
            // check empty -> non-empty transition
            if ( queue_.size() == 1 ) { fulfill( queue_.front() ); }
            return queue_.back().second.get_future();
        }
        
        // unblock placeholder, fulfill next promise if present
        void remove( ThdId id )
        {
            SpinLock    _lock(flag_);
            
            if ( curr_ != id ) { throw std::runtime_error("release by wrong thread"); }
            
            if ( --count_ == 0 )
            {
                queue_.pop_front();
                if ( queue_.empty() ) { curr_ = ThdId(); }
                else                  { fulfill( queue_.front() ); }
            }
        }
        
        WaitQueue(WaitQueue const&) = delete;
        WaitQueue& operator=( WaitQueue ) = delete;
    };
    
    class Ticket
    {
        WaitQueue&  wq_;
    public:
        explicit Ticket(WaitQueue& wq) : wq_(wq) { wq.lock(); }
        ~Ticket() noexcept { wq_.unlock(); }
    };
    
} //  namespace Utility   
