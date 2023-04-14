
#include "LogImpl.h"

	/**
	 * @file LogImplStub.cpp
	 * @brief Do-nothing implementation
	 */

namespace LogImpl
{
	void initialize( char const* ) {}
	void finalize() {}
	void* acquire_pimpl( char const*, Utility::Log::Level ) { return nullptr; }
	void  release_pimpl( void* ) {}
	Utility::Log::Level get_level( void* ) { return Utility::Log::get_global_level(); }
	Utility::Log::Level set_level( void*, Utility::Log::Level level ) { return level; }
	void* is_active( void* ptr, Utility::Log::Level level ) { return ptr; }
	void commit( void*, Utility::Log::Level, Utility::Location const&, char const* ) {}

} // namespace LogImpl
