#pragma once

#include <atomic>
#include <thread>

namespace Utility
{
    /**
     * FlagLock.
     * Basic Lockable type adapter for an atomic_flag. 
     */
    class FlagLock
    {
        std::atomic_flag    flag_{ATOMIC_FLAG_INIT};

    public:
        void lock()
        {
            while ( flag_.test_and_set() )
            {
                std::this_thread::yield();
            }
        }

        void unlock()
        {
            flag_.clear();
        }
    };

} // namespace Utility
