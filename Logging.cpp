
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

#define ENUM2STR(lvl, ...)	case Log::level::lvl : return #lvl;
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
	: Logger(name, Log::get_log_level())
	{}
	Logger::Logger(char const* name, Log::Level level)
	: name_(name)
	, impl_(LogImpl::acquire_pimpl( name, level ))
	, level_(impl_ ? LogImpl::get_level( impl_ ) : level)
	{}

	Log::Token
	Logger::is_enabled( Log::Level level )
	{
		auto	_tok(LogImpl::is_enabled( impl_, level ));
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
		std::oftsream	ofs;
		std::ostream*	outp = &std::cerr;

#define STR2ENUM(lvl, ...)	{ #lvl, Log::Level::lvl }.

		std::map<std::string, Log::Level>	levelMap =
		{
			LOG_LEVELS(STR2ENUM)
		};

		//default, 
		void default_commit( char const* level, Location const& loc, char const* msg )
		{
		}
	} // namespace anonymous

	bool
	Log::set_default_log( char const* filename, bool append )
	{
	}

	bool
	Log::set_log_level( std::string const& level )
	{
		auto	_itr(levelMap.find( level ));
		if ( _itr == levelMap.end() ) { return false; }
		globalLevel = _itr.second;
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

} // namespace Utility

