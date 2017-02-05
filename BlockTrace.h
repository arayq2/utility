#pragma once

#include "Logger.h"

namespace Log
{
    // BlockTrace
    // Sentinel object to log entry into and exit from blocks
    //
    class BlockTrace
    {
    public:
        BlockTrace(char const* bn = "(no name)")
        : bn_(bn)
        {
            DebugLog0 << "ENTER: " << bn_;
        }
        
        ~BlockTrace() noexcept
        {
            DebugLog0 << "LEAVE: " << bn_;
        }
        
    private:
        char const*     bn_;
    };

} // namespace Log

#define FUNCTION_TRACE() \
    Log::BlockTrace     _function_tracer_(__FUNCTION__)
