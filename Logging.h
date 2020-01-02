#pragma once

#ifndef UTILITY_LOGGING_H
#define UTILITY_LOGGING_H

#include "Log.h"

#define LOCATION()  Utility::Location({__FUNCTION__, __FILE__, __LINE__})

// For message construction using insertion syntax. Example:
//    LOG_STREAM(logger, Utility::Log::Level::INFO, "my message: " << myarg);
//
#define LOG_STREAM(L, P, X) \
    do if ( Utility::Log::Token _token{Utility::Log::is_active( L, P )} ) { \
        LOG_COMMIT_STREAM(_token, X); \
    } while ( false )

#define LOG_STRM_DEBUG(L, X) LOG_STREAM(L, Utility::Log::Level::DEBUG, X)
#define LOG_STRM_INFO(L, X)  LOG_STREAM(L, Utility::Log::Level::INFO, X)
#define LOG_STRM_WARN(L, X)  LOG_STREAM(L, Utility::Log::Level::WARN, X)
#define LOG_STRM_ERROR(L, X) LOG_STREAM(L, Utility::Log::Level::ERROR, X)
#define LOG_STRM_FATAL(L, X) LOG_STREAM(L, Utility::Log::Level::FATAL, X)

// Token can be pre-acquired for multiple calls
#define LOG_STREAM_IF(T, X)   \
    do if ( T ) { \
        LOG_COMMIT_STREAM(T, X); \
    } while ( false )

#include <sstream>
#define LOG_COMMIT_STREAM(T, X) \
    std::ostringstream  _oss; \
    _oss << X; \
    Utility::Log::commit( T, LOCATION(), _oss.str().c_str() )

// For message construction using printf syntax. Example:
//    LOG_FORMAT(logger, Utility::Log::Level::INFO, "my message: %s", myarg);
//
#define LOG_FORMAT(L, P, ...) \
    do if ( Utility::Log::Token _token{Utility::Log::is_active( L, P )} ) { \
        LOG_COMMIT_FORMAT(_token, __VA_ARGS__); \
    } while ( false )

#define LOG_FMT_DEBUG(L, ...) LOG_FORMAT(L, Utility::Log::Level::DEBUG, __VA_ARGS__)
#define LOG_FMT_INFO(L, ...)  LOG_FORMAT(L, Utility::Log::Level::INFO, __VA_ARGS__)
#define LOG_FMT_WARN(L, ...)  LOG_FORMAT(L, Utility::Log::Level::WARN, __VA_ARGS__)
#define LOG_FMT_ERROR(L, ...) LOG_FORMAT(L, Utility::Log::Level::ERROR, __VA_ARGS__)
#define LOG_FMT_FATAL(L, ...) LOG_FORMAT(L, Utility::Log::Level::FATAL, __VA_ARGS__)

// Token can be pre-acquired for multiple calls
#define LOG_FORMAT_IF(T, ...)   \
    do if ( T ) { \
        LOG_COMMIT_FORMAT(T, __VA_ARGS__); \
    } while ( false )

#include "CharBuffer.h"
#define LOG_COMMIT_FORMAT(T, ...) \
    using Buffer = Utility::CharBuffer<2048>; \
    Utility::Log::commit( T, LOCATION(), Buffer(__VA_ARGS__).get() )

#endif // UTILITY_LOGGING_H
