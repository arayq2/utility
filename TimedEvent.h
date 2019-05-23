#pragma once

#include "SpinLock.h"
#include <future>

namespace Utility
{
    /**
     * TimedEvent.
     * Notification subject to a timeout.  Notifier is returned 
     * expiration status, waiter is returned completion status.
     *
     * Intended use is conditional transfer of responsibility 
     * (e.g., for resource release) from notifier to waiter: if
     * the wait period has expired, responsibility remains with
     * the notifier; otherwise, it is transfered to the waiter. 
     */
    class TimedEvent
    {
    public:
        TimedEvent() = default;
        ~TimedEvent() noexcept = default;

        // called by notifier (producer)
        bool cancelled()
        {
            SpinLock    _lock(flag_);
            completed_ = true;
            try { promise_.set_value(); }
            catch ( std::future_error& ) {}
            return cancelled_;
        }
        // called by waiter (consumer)
        bool completed( unsigned millis )
        {
            promise_.get_future().wait_for( std::chrono::milliseconds(millis) );
            SpinLock    _lock(flag_);
            cancelled_ = true;
            return completed_;
        }
        
        bool reset()
        {
            SpinLock    _lock(flag_);
            if ( !cancelled_ or !completed_ ) { return false; }
            promise_ = std::promise<void>();
            return true;
        }

    private:
        std::atomic_flag    flag_{ATOMIC_FLAG_INIT};
        bool                completed_{false};
        bool                cancelled_{false};
        std::promise<void>  promise_;
    };

} // namespace Utility

