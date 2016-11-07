
#define LOGGER_DECL
#include "Logger.h"
#include "Lockable.h"
#include "TimeStamp.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

namespace Log
{

namespace // underlying implementation, not exposed
{

    char const* STDERR_TAG = "##StdErr##";
    
    inline
    char const* log_level_string( ulong level )
    {
        switch ( level )
        {
        case LEVEL_DEBUG:   return "DEBUG";
        case LEVEL_INFO:    return "INFO ";
        case LEVEL_WARN:    return "WARN ";
        case LEVEL_ERROR:   return "ERROR";
        case LEVEL_FATAL:   return "FATAL";
        default:            return "FIXTHIS!";
        }
    }

    inline
    void stream_print( std::ostream& os, std::ostringstream& msg, std::string const& timestamp, char const* strLevel )
    {
        os << timestamp
           << "|" << strLevel
           << "|" << std::setw(5) << ::syscall( SYS_gettid )
           << "|" << msg.str()
           << std::endl;
    }

    /**
     * LoggerBase:
     * aggregated by LogManager (see below)
     */
    class LoggerBase
    {
    public:
        LoggerBase(char const* name, ulong mask)
        : name_(name)
        , mask_(mask)
        {}
        
        virtual ~LoggerBase() {}
        
        bool is_active( ulong level ) const { return mask_ & level; }
        
        bool is_named( char const* name ) const { return name_.compare( name ) == 0; }

        void write_log( std::ostringstream& msg, std::string const& ts, char const* strLevel, ulong level )
        {
            if ( mask_ & level ) { do_write( msg, ts, strLevel ); }
        }
        
        
    private:
        std::string     name_;
        ulong           mask_;
        
        virtual void do_write( std::ostringstream& msg, std::string const& ts, char const* strLevel ) {}
    };
    
    // Various subclass implemnentations
    
    // log to STDERR, used internally as a default logger
    class StdErrLog
    : public LoggerBase
    {
    public:
        StdErrLog(ulong mask = LEVEL_ALL)
        : LoggerBase(STDERR_TAG, mask)
        {}
        
        virtual void
        do_write( std::ostringstream& msg, std::string const& ts, char const* strLevel ) override
        {
            stream_print( std::cerr, msg, ts, strLevel );
        }
    };
    
    // log to file
    class FileLog
    : public LoggerBase
    {
    public:
        FileLog(std::string const& file, char const* tag = "File", ulong mask = LEVEL_ALL)
        : LoggerBase(tag, mask)
        , ofs_(file.c_str())
        {}
        
        virtual void
        do_write( std::ostringstream& msg, std::string const& ts, char const* strLevel ) override
        {
            stream_print( ofs_, msg, ts, strLevel );
        }
        
    private:
        std::ofstream   ofs_;
    };
    
    // log to stream
    class StreamLog
    : public LoggerBase
    {
    public:
        StreamLog(std::ostream& os, char const* tag = "Stream", ulong mask = LEVEL_ALL)
        : LoggerBase(tag, mask)
        , os_(os)
        {}
        
        virtual void
        do_write( std::ostringstream& msg, std::string const& ts, char const* strLevel ) override
        {
            stream_print( os_, msg, ts, strLevel );
        }
        
    private:
        std::ostream&   os_;
    };
    
    // pass log entry to external handler
    class FunctorLog
    : public LoggerBase
    {
    public:
        FunctorLog(LogStream::Functor&& functor, char const* tag = "Functor", ulong mask = LEVEL_ALL)
        : LoggerBase(tag, mask)
        , functor_(std::move(functor))
        {}
    
        virtual void
        do_write( std::ostringstream& msg, std::string const& ts, char const* strLevel ) override
        {
            functor_( msg.str().c_str(), ts.c_str(), strLevel );
        }

    private:
        LogStream::Functor  functor_;
    };

    // pass log entry to a function
    
    class CallbackLog
    : public LoggerBase
    {
    public:
        CallbackLog(LogStream::Callback callback, void* context, char const* tag = "Callback", ulong mask = LEVEL_ALL)
        : LoggerBase(tag, mask)
        , callback_(callback)
        , context_(context)
        {}
        
        virtual void
        do_write( std::ostringstream& msg, std::string const& ts, char const* strLevel ) override
        {
            callback_( context_, msg.str().c_str(), ts.c_str(), strLevel );
        }
        
    private:
        LogStream::Callback     callback_;
        void*                   context_;
    };
    
    /**
     * LogManager:
     * API implementation.
     */
    class LogManager
    : private Utility::Relockable
    {
        /**
         * Helper class to prevent reentry:
         * toggles an initial false to true and back again.
         */
        class ToggleFalse
        {
        public:
            ToggleFalse(bool& value)
            : value_(value)
            , ok_(!value)
            {
                if ( ok_ ) { value_ = true; }
            }
            
            ~ToggleFalse() { if ( ok_ ) { value_ = false; } }
            
            operator bool() const { return ok_; }
        
        private:
            bool&   value_;
            bool    ok_;
        };
        
    public:
        LogManager()
        : loggers_()
        , ts_()
        , reentry_(false)
        , first_(false)
        {
            loggers_.push_back( new StdErrLog );
        }
        
        ~LogManager() noexcept
        {
            for ( auto logger : loggers_ ) { delete logger; }
            loggers_.clear();
        }
        
        void add_logger( LoggerBase* logger )
        {
            AUTORELOCK();
            if ( !first_ )
            {
                remove_loggers( STDERR_TAG ); // kill logger from ctor
                first_ = true;
            }
            loggers_.push_back( logger );
        }
     
        void remove_loggers( char const* tag )
        {
            AUTORELOCK();
            loggers_.erase( std::remove_if( loggers_.begin(), loggers_.end(),
            [tag]( LoggerBase* logger )
            {
                if ( logger->is_named( tag ) )
                {
                    delete logger;
                    return true;
                }
                return false;
            } ), loggers_.end() );
        }
        
        void write_log( std::ostringstream& msg, ulong level )
        {
            std::string const       _now(ts_.reset());
            char const*             _level(log_level_string( level ));              
            AUTORELOCK();
            if ( ToggleFalse _check{reentry_} )
            {
                for ( auto logger : loggers_ )
                {
                    logger->write_log( msg, _now, _level, level );
                }
            }
        }
        
    private:
        std::vector<LoggerBase*>    loggers_;
        Utility::TimeStamp          ts_;
        bool                        reentry_;
        bool                        first_;
     };
     
    //avoid static initialization order disasters
    LogManager& log_manager()
    {
        static LogManager   _manager;
        return _manager;
    }
    
} // anonymous namespace

// API Section

    LogStream::~LogStream() noexcept
    {
        try
        {
            log_manager().write_log( oss_, level_ );
        }
        catch (...) {}
    }

    void LogStream::add_file_logger( char const* file, ulong mask, char const* tag )
    {
        log_manager().add_logger( new FileLog( file, tag, mask ) );
    }
    
    void LogStream::add_stream_logger( std::ostream& stream, ulong mask, char const* tag )
    {
        log_manager().add_logger( new StreamLog( stream, tag, mask ) );
    }
    
    void LogStream::add_functor_logger( Functor&& functor, ulong mask, char const* tag )
    {
        log_manager().add_logger( new FunctorLog( std::move(functor), tag, mask ) );
    }
    
    void LogStream::add_callback_logger( Callback callback, void* context, ulong mask, char const* tag )
    {
        log_manager().add_logger( new CallbackLog( callback, context, tag, mask ) );
    }
    
    void LogStream::remove_loggers( char const* tag )
    {
        log_manager().remove_loggers( tag );
    }

    void LogStream::stop_default_logger()
    {
        log_manager().remove_loggers( STDERR_TAG );
    }


} // namespace Log
