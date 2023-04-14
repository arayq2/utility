#pragma once

#ifndef LOGIMPL_H
#define LOGIMPL_H

#include "Log.h"

namespace LogImpl
{
	extern void initialize( char const* );
	extern void finalize();
	//
	extern void* acquire_pimpl( char const* name, Utility::Log::Level level );
	extern void  release_pimpl( void* pimpl );
	//
	extern Utility::Log::Level get_level( void* pimpl );
	extern Utility::Log::Level set_level( void* pimpl, Utility::Log::Level level );
	//
	using Tok = void*;
	extern Tok is_active( void* pimpl, Utility::Log::Level level );
	extern void commit( Tok tok, Utility::Log::Level level, Utility::Location const& loc, char const* msg );
} // namespace LogImpl

#endif //  LOGIMPL_H
