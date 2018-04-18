
#pragma once

#include "Logger.h"
#include <stdexcept>

namespace Utility
{
    // BoolException.
    // Decorate a boolean expression with an exception on mismatch
    //
    template<bool ONTRUE, typename Exception = std::runtime_error>
    class BoolException
    {
    public:
        BoolException(Location const& location, bool condition, char const* ctxt ="Unspecified context")
        : condition_(condition)
        {
            if ( ONTRUE == condition )
            {
                ErrorLog0 << location << "Throwing exception: " << ctxt;
                throw Exception(ctxt);
            }
            DebugLog0 << location << "Status OK: " << ctxt;
        }
        
        operator bool () const { return condition_; }
    private:
        bool    condition_;
    };

} // namespace Utility

#define THROW_UNLESS(...)       THROW_ON_FALSE(std::runtime_error,__VA_ARGS__)
#define THROW_IF(...)           THROW_ON_TRUE(std::runtime_error,__VA_ARGS__)
#define THROW_ON_FALSE(X,...)   Utility::BoolException<false, X>(LOCATION(),__VA_ARGS__) 
#define THROW_ON_TRUE(X,...)    Utility::BoolException<true, X>(LOCATION(),__VA_ARGS__) 
    

