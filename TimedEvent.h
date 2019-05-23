#pragma once

#include "FlipFlop.h"
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
        bool is_cancelled()
        {
            if ( switch_.flip() ) { return true; }
            try { promise_.set_value(); } catch ( std::future_error& ) {}
            return false;
        }
        // called by waiter (consumer)
        bool is_completed( unsigned millis )
        {
            promise_.get_future().wait_for( std::chrono::milliseconds(millis) );
            return switch_.flop();
        }
        
        bool reset()
        {
            if ( !switch_.reset() ) { return false; }
            promise_ = std::promise<void>();
            return true;
        }

    private:
        std::promise<void>  promise_;
        FlipFlop            switch_;
    };

} // namespace Utility

