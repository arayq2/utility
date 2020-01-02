#pragma once

#ifndef UTILITY_LOG_H
#define UTILITY_LOG_H

#include <cstdint>
#include <ostream>

    /**
     * @file Log.h
     * @brief Front-end classes for logging.
     * Implementation is deliberately hidden at this level.
     * Usage is through macros.
     * @see Logging.h
     */
namespace Utility
{
//==========================================================================
    struct Location
    {
        char const* func_;
        char const* file_;
        uint32_t    line_;

        std::ostream& output( std::ostream& ) const;
        friend
        std::ostream& operator<<( std::ostream& os, Location const& loc )
        {
            return loc.output( os );
        }
    };

//==========================================================================

    class Logger; // forward declaration to untangle mutual dependencies

    class Log
    {
    public:
        enum class Level
        : uint32_t
        {
            ALL,
            DEBUG, INFO, WARN, ERROR, FATAL,
            OFF
        };

        struct Token
        {
            //char const* name_{nullptr};
            void*       impl_{nullptr};
            Level       level_{Level::OFF};

            explicit operator bool() const { return nullptr != impl_; }
        };

        static Token is_active( Logger const*, Log::Level ); 
        static Token is_active( Logger const&, Log::Level );
        //
        static Level get_global_level() { return globalLevel; }
        static void set_global_level( Level lvl ) { globalLevel = lvl; }
        static bool is_active( Level lvl ) { return lvl >= globalLevel; }
        static char const* level_string( Level lvl );
        //
        static void commit( Token const&, Location const&, char const* );

    private:
        static Level    globalLevel;
    };

//==========================================================================

    class Logger
    {
    public:
        Logger(char const* name); // use default level
        Logger(char const* name, Log::Level level);

        Logger& set_level( Log::Level lvl ) { level_ = lvl; return *this; }

        Log::Token is_active( Log::Level lvl ) const;

    private:
        char const*     name_;
        void*           impl_{nullptr}; // implementation
        Log::Level      level_;
        friend class Log;
    };

//==========================================================================

    inline Log::Token
    Log::is_active( Logger const* logger, Log::Level level )
    {
        return logger ? logger->is_active( level ) : Log::Token();
    }

    inline Log::Token
    Log::is_active( Logger const& logger, Log::Level level )
    {
        return logger.is_active( level );
    }

} // namespace Utility

#endif // UTILITY_LOG_H
