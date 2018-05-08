#pragma once

#include <atomic>
#include <thread>

namespace Utility
{
    /**
     * SpinLock.
     * Set and clear an atomic_flag; used as a scope guard. 
     */
    class SpinLock
    {
        std::atomic_flag&    flag_;

    public:
        explicit
        SpinLock(std::atomic_flag& flag)
        : flag_(flag)
        {
            while ( flag.test_and_set() )
            {
                std::this_thread::yield();
            }
        }

        ~SpinLock() noexcept
        {
            flag_.clear();
        }
    };

} // namespace Utility
