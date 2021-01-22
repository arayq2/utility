
#include "LogImpl.h"
#include "CharBuffer.h"
#include "TimeFns.h"

#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/syscall.h>

namespace Utility
{
	std::ostream&
	Location::output( std::ostream& os ) const
	{
		return os << "[" << func_ << "@_" << file_ << ":" << line_ << "]";
	}

	std::string
	Location::to_string() const
	{
		std::ostringstream	_oss;
		output( _oss );
		return _oss.str();
	}

#define ENUM2STR(lvl, ...)	case Log::Level::lvl : return #lvl;
	char const*
	Log::level_string( Log::Level const& level )
	{
		switch (level)
		{
		default:
		LOG_LEVELS(ENUM2STR);
		}
	}

	Logger::~Logger()
	{
		LogImpl::release_pimpl( impl_ );
	}

	Logger::Logger(char const* name)
	: Logger(name, Log::get_global_level())
	{}
	Logger::Logger(char const* name, Log::Level level)
	: name_(name)
	, impl_(LogImpl::acquire_pimpl( name, level ))
	, level_(impl_ ? LogImpl::get_level( impl_ ) : level)
	{}

	Log::Token
	Logger::is_active( Log::Level level ) const
	{
		auto	_tok(LogImpl::is_active( impl_, level ));
		return {_tok, (_tok == nullptr ? Log::Level::OFF : level)};
	}

	Logger&
	Logger::set_level( Log::Level level )
	{
		level_ = LogImpl::set_level( impl_, level );
		return *this;
	}

	namespace
	{
		std::ofstream	ofs;
		std::ostream*	outp = &std::cerr;

#define STR2ENUM(lvl, ...)	{ #lvl, Log::Level::lvl },

		std::map<std::string, Log::Level>	levelMap =
		{
			LOG_LEVELS(STR2ENUM)
		};

		//default, 
		void default_commit( char const* level, Location const& loc, char const* msg )
		{
            LocalTime   _now;
            // Format: Timestamp|Level|ID|Message (Location)
            *outp << Utility::CharBuffer<2048>("%4d-%02d-%02dT%02d:%02d:%02d.%06ld|%5s|%6d|%s (%s@%s:%d)\n"
                , (_now.tm_.tm_year + 1900), (_now.tm_.tm_mon + 1), _now.tm_.tm_mday
                , _now.tm_.tm_hour, _now.tm_.tm_min, _now.tm_.tm_sec, (_now.ts_.tv_nsec /1000)
                , level
                , ::syscall( SYS_gettid )
                , msg
                , loc.func_, loc.file_, loc.line_)
                .get();
		}
	} // namespace anonymous

	bool
	Log::set_default_log( char const* filename, bool append )
	{
        if ( filename )
        {
            // Should improve this to use direct swap.
            ofs.close();
            ofs.open( filename, append ? std::ios_base::out | std::ios_base::app : std::ios_base::out );
            if ( ofs )
            {
                outp = &ofs;
                return true;
            }
            outp = &std::cerr;
            return false;
        }
        else // restore default
        {
            ofs.close();
            outp = &std::cerr;
        }
        return true;
	}

	bool
	Log::set_log_level( std::string const& level )
	{
		auto	_itr(levelMap.find( level ));
		if ( _itr == levelMap.end() ) { return false; }
		globalLevel = _itr->second;
		return true;
	}

	void
	Log::commit( Log::Token const& token, Location const& loc, char const* msg )
	{
		if ( token.impl_ )
		{
			LogImpl::commit( token.impl_, token.level_, loc, msg );
		}
		else
		{
			default_commit( level_string( token.level_ ), loc, msg );
		}
	}

    Log::Level Log::globalLevel = Log::Level::INFO;
} // namespace Utility

