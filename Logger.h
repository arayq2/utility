#pragma once

#ifndef LOGGER_DECL
#define LOGGER_DECL extern
#endif

    /**
     * Logger:
     * Thread-safe logger supporting stream insertion syntax to compose
     * entries. Usage is like this:
     *
     *      InfoLog << some_stuff << more_stuff << etc;
     *
     * (Note: std::endl should NOT be inserted; flushing is handled 
     * internally.)
     * Supported levels are: Debug, Info, Warn, Error, and Fatal.
     *
     * DebugLog, InfoLog, WarnLog, ErrorLog and FatalLog are macros that 
     * expose a std::ostringstream - which is why the stream insertion 
     * syntax works - member of a temporary wrapper object.  When the 
     * object goes out of scope at the end of the statement, the dtor
     * takes care of committing the log entry to an underlying thread
     * safe implementation.  This also means that a logger can be passed
     * to functions as an argument of type std::ostream&. 
     */

#include "Location.h"
#include <sstream>
#include <functional>

namespace Log
{

    enum LogLevel
    {
        LEVEL_DEBUG = 0x01,
        LEVEL_INFO  = 0x02,
        LEVEL_WARN  = 0x04,
        LEVEL_ERROR = 0x08,
        LEVEL_FATAL = 0x10,
        LEVEL_ALL   = 0x1f
    };
    
    enum
    {
        FILE_LOGGING  = ~LEVEL_DEBUG & LEVEL_ALL,
        EVENT_LOGGING = LEVEL_ERROR & LEVEL_FATAL
    };
    
    using Utility::Location;
    
    
    /**
     * LogStream:
     * Accumulate items into a local stringstream buffer until end 
     * of scope, when dtor will commit the entry to the underlying 
     * threadsafe API.
     */
    class LogStream 
    {
    public:
        using ulong = unsigned long;
        LogStream(Location const& location, ulong level)
        : oss_()
        , level_(level)
        {
            oss_ << location << "|";
        }
        
        LogStream(ulong level)
        : oss_()
        , level_(level)
        {}
        
        ~LogStream() noexcept; // all the action is here
        
        // see macros below
        std::ostream& log() { return oss_; }
        
        // management API: the optional tag is for identification and/or grouping.
        static void add_file_logger( char const* file, ulong mask = FILE_LOGGING, char const* tag = "File" );
        static void add_stream_logger( std::ostream& stream, ulong mask = FILE_LOGGING, char const* tag = "Stream" );
        
        // delegation to user defined logging methods
        using Functor = std::function<void(char const* msg, char const* timestamp, char const* level)>;
        static void add_functor_logger( Functor&& functor, ulong mask = FILE_LOGGING, char const* tag = "Functor" );
        
        using Callback = void (*)( void* context, char const* msg, char const* timestamp, char const* level );
        static void add_callback_logger( Callback callback, void* context, ulong mask = FILE_LOGGING, char const* tag = "Callback" );
        
        // removes all loggers for a given tag
        static void remove_loggers( char const* tag );
        // disables a default logger, statically initialized to enable logging before main() is entered.
        static void stop_default_logger();
        
    private:
        std::ostringstream      oss_;
        ulong                   level_;
    };
    
    class NullStream
    : public std::ostream
    {
    public:
        NullStream()
        : std::ostream(&nullBuf_)
        {}
        
    private:
        struct NullBuf
        : public std::streambuf
        {
            int overflow( int c ) { return c; }
        }       nullBuf_;
    };

} // namespace Log

#ifdef NOLOGGING

LOGGER_DECL Log::NullStream     DebugLog;
LOGGER_DECL Log::NullStream     InfoLog;
LOGGER_DECL Log::NullStream     WarnLog;
LOGGER_DECL Log::NullStream     ErrorLog;
LOGGER_DECL Log::NullStream     FatalLog;

LOGGER_DECL Log::NullStream     DebugLog0;
LOGGER_DECL Log::NullStream     InfoLog0;
LOGGER_DECL Log::NullStream     WarnLog0;
LOGGER_DECL Log::NullStream     ErrorLog0;
LOGGER_DECL Log::NullStream     FatalLog0;

#else

#define LOCATION()  Utility::Location(__FUNCTION__, __FILE__, __LINE__)

#define DebugLog    Log::LogStream(LOCATION(), Log::LEVEL_DEBUG).log()
#define InfoLog     Log::LogStream(LOCATION(), Log::LEVEL_INFO).log()
#define WarnLog     Log::LogStream(LOCATION(), Log::LEVEL_WARN).log()
#define ErrorLog    Log::LogStream(LOCATION(), Log::LEVEL_ERROR).log()
#define FatalLog    Log::LogStream(LOCATION(), Log::LEVEL_FATAL).log()

#define DebugLog0   Log::LogStream(Log::LEVEL_DEBUG).log()
#define InfoLog0    Log::LogStream(Log::LEVEL_INFO).log()
#define WarnLog0    Log::LogStream(Log::LEVEL_WARN).log()
#define ErrorLog0   Log::LogStream(Log::LEVEL_ERROR).log()
#define FatalLog0   Log::LogStream(Log::LEVEL_FATAL).log()

#endif
