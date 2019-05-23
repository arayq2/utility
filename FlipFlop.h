#pragma once

#include "SpinLock.h"

namespace Utility
{

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
            flop_ = true;
            return flip_;
        }

        bool flop()
        {
            SpinLock    _lock(flag_);
            flip_ = true;
            return flop_;
        }

        bool reset()
        {
            SpinLock    _lock(flag_);
            if ( !flip_ or !flop_ ) { return false; }
            return true;
        }

    private:
        std::atomic_flag    flag_{ATOMIC_FLAG_INIT};
        bool                flip_{false};
        bool                flop_{false};
    };

} // namespace Utility
