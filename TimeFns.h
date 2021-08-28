/** ======================================================================+
 + Copyright @2018-2021 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#pragma once

#ifndef UTILITY_TIMEFNS_H
#define UTILITY_TIMEFNS_H

#include "CharBuffer.h"
#include <chrono>
#include <utility>
#include <unistd.h>
#include <time.h>

namespace Utility
{
	struct LocalTime
    {
        struct timespec     ts_;
        struct tm           tm_;

        LocalTime()
        {
            ::clock_gettime( CLOCK_REALTIME, &ts_ );
            ::localtime_r( &ts_.tv_sec, &tm_ );
        }

        long current_date() const
        {
            return 10000 * (tm_.tm_year + 1900) + 100 * (tm_.tm_mon + 1) + tm_.tm_mday;
        }

        std::string current_datetime() const
        {
            return Utility::CharBuffer<32>("%d/%d/%d, %d:%02d:%02d"
                , (tm_.tm_mon +1 ), tm_.tm_mday, (tm_.tm_year + 1900)
                , tm_.tm_hour, tm_.tm_min, tm_.tm_sec).get();
        }
    };

    static inline
    long current_date()
    {
        return LocalTime().current_date();
    }

    static inline
    std::string current_datetime()
    {
        return LocalTime().current_datetime();
    }

    struct MilliTimer
    {
        using Clock     = std::chrono::steady_clock;
        using TimePoint = Clock::time_point;
        using Millisecs = std::chrono::milliseconds;

        TimePoint   start_;

        MilliTimer()
        : start_(Clock::now())
        {}

        long long mark() const
        {
            return std::chrono::duration_cast<Millisecs>(Clock::now() - start_).count();
        }

        template<typename Fn, typename... Args>
        static
        long long time( Fn&& fn, Args&&... args )
        {
            MilliTimer  _timer;
            fn( std::forward<Args>(args)... );
            return _timer.mark();
        }
    };

} // namespace Utility

#endif // UTILITY_TIMEFNS_H

