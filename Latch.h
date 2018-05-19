#pragma once

#include "SpinLock.h"
#include <future>
#include <stdexcept>

namespace Utility
{
    /**
     * Latch.
     * Countdown latch using a shared future.
     */
    class Latch
    {
        using Promise = std::promise<void>; // producer of ready signal
        using Future  = std::shared_future<void>;  // consumer of ready signal
        using Flag    = std::atomic_flag; 
    public:
        explicit
        Latch(size_t count = 1)
        : count_(count)
        , promise_()
        , future_(promise_.get_future())
        {}
        
        void wait() { acquire().get(); }
        
    private:
        size_t      count_;
        Promise     promise_;
        Future      future_;
        Flag        flag_{ATOMIC_FLAG_INIT};
        
        Future acquire()
        {
            SpinLock    _lock(flag_);
            
            if ( --count_ == 0 ) { promise_.set_value(); }
            return Future(future_);
        }
    };
} // namespace Utility
