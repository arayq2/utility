#pragma once

#include <time.h>
#include <ostream>
#include <string>
#include <cstdio>

namespace Utility
{
    class TimeStamp
    {
    public:
        TimeStamp() { init_(); }
        
        TimeStamp& reset()
        {
            init_();
            return *this;
        }
        
        operator std::string const() const { return std::string(buffer_); }
        
        std::ostream& output( std::ostream& os ) const { return os << buffer_; }
    
    private:
        enum { TSBUFSIZ = 28 };
        
        struct timespec     ts_;
        char                buffer_[TSBUFSIZ];
        
        void init_()
        {
            ::clock_gettime( CLOCK_REALTIME, &ts_ );
            set_();
        }
        
        void set_()
        {
            struct tm   _tm;
            ::localtime_r( &ts_.tv_sec, &_tm );
            
            std::sprintf( buffer_, "%4d-%02d-%02dT%02d:%02d:%02d.%06ld"
                , (_tm.tm_year + 1900)
                , (_tm.tm_mon + 1)
                , _tm.tm_mday
                , _tm.tm_hour
                , _tm.tm_min
                , _tm.tm_sec
                , (ts_.tv_nsec / 1000)
                );
        }
    };
    
    inline
    std::ostream& operator<< ( std::ostream& os, TimeStamp const& timestamp )
    {
        return timestamp.output( os );
    }
    
} // namespace Utility
