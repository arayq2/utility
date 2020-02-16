#pragma once

#include "SpinLock.h"

namespace Utility
{

    /**
     * FlipFlop.
     * Race condition arbitration between two cooperating events, 
     * represented as boolean transitions from false to true.
     *
     * Each event is informed of the status of the other. Exactly 
     * one of the calls to flip() and flop() will return false
     * (the race winner), and the other, true (the loser).
     */
    class FlipFlop
    {
    public:
        FlipFlop() = default;
        ~FlipFlop() noexcept = default;
        FlipFlop(FlipFlop const&) = delete;
        FlipFlop& operator=( FlipFlop const& ) = delete;

        bool flip()
        {
            SpinLock    _lock(flag_);
            flip_ = true;
            return flop_;
        }

        bool flop()
        {
            SpinLock    _lock(flag_);
            flop_ = true;
            return flip_;
        }

        bool reset( bool force = false )
        {
            SpinLock    _lock(flag_);
            if ( !force and (!flip_ or !flop_) ) { return false; }
            flip_ = flop_ = false;
            return true;
        }

    private:
        std::atomic_flag    flag_{ATOMIC_FLAG_INIT};
        bool                flip_{false};
        bool                flop_{false};
    };

} // namespace Utility
