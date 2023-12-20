/** ======================================================================+
 + Copyright @2020-2021 Arjun Ray
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
        struct timespec ts_;
        struct tm       tm_;

        LocalTime()
        {
            ::clock_gettime( CLOCK_REALTIME, &ts_ );
            ::localtime_r( &ts_.tv_sec, &tm_ );
        }

        explicit
        LocalTime(long secs, long nano = 0L)
        : ts_({ secs, nano })
        {
            ::localtime_r( &ts_.tv_sec, &tm_ );
        }

        long current_date() const
        {
            return 10000 * (tm_.tm_year + 1900) + 100 * (tm_.tm_mon + 1) + tm_.tm_mday;
        }

        std::string current_datetime() const
        {
            return Utility::CharBuffer<32>("%d/%d/%d, %d:%02d:%02d"
            , (tm_.tm_mon + 1), tm_.tm_mday, (tm_.tm_year + 1900) 
            , tm_.tm_hour, tm_.tm_min, tm_.tm_sec).get();
        }

        // wraps a call to strftime, providing the formatting spec
        std::string format( char const* spec ) const
        {
            return Utility::CharBuffer<128>().apply( ::strftime, spec, &tm_ ).get();
        }
    };

    static inline
    long current_date()
    {
        return LocalTime().current_date();
    }

    static inline
    long to_date( long millisecs )
    {
        return LocalTime(millisecs / 1000).current_date();
    }

    static inline
    std::string current_datetime()
    {
        return LocalTime().current_datetime();
    }

    static inline
    std::string to_datetime( long millisecs )
    {
        return LocalTime(millisecs / 1000).current_datetime();
    }

    template<typename Clock = std::chrono::system_clock>
    struct MilliTimer
    {
        using TimePoint = typename Clock::time_point;
        using MilliSecs = std::chrono::milliseconds;

        TimePoint   start_;

        MilliTimer()
        : start_(Clock::now())
        {}

        long long mark() const
        {
            return std::chrono::duration_cast<MilliSecs>(Clock::now() - start_).count();
        }

        long long reset()
        {
            start_ = Clock::now();
            return std::chrono::duration_cast<MilliSecs>(start_.time_since_epoch()).count();
        }

        template<typename Fn, typename... Args>
        static
        long long time( Fn&& fn, Args&&... args )
        {
            MilliTimer  _mt;
            fn( std::forward<Args>(args)... );
            return _mt.mark();
        }
    };

} // namespace Utility

#endif // UTILITY_TIMEFNS_H
