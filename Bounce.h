#pragma once

#include <utility>  // std::forward

namespace Utility
{
    /**
     * Bounce.
     * Convert callback-style free functions into method calls
     * on an object (class) provided in the argument list. 
     * This saves on boilerplate code and avoids name clutter.
     */
    template<typename To, typename From, typename... Args>
    struct Bounce
    {
        static To* to_ptr( From* from )
        {
            return reinterpret_cast<To*>(from);
        }

        template<typename Retval, Retval (To::* Method)(Args...)>
        static Retval rv_fn( From* from, Args... args )
        {
            return (to_ptr( from )->*Method)( std::forward<Args>(args)... );
        }

        template<void (To::* Method)(Args...)>
        static void vd_fn( From* from, Args... args )
        {
            (to_ptr( from )->*Method)( std::forward<Args>(args)... );
        }
    };

} // namespace Utility
