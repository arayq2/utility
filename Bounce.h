#pragma once

#include <utility>  // std::forward

namespace Utility
{
    /**
     * Bounce.
     * Convert callback-style free functions into method calls
     * on an object (class) provided in the argument list. 
     * This saves on boilerplate code and avoids name clutter.
     * For simple pointer recasting cases, the usage will be
     * like this:
     * 
     *     using MyCaster = Caster<MyTo, MyFrom>;
     *
     *     template<typename... Args>
     *     using MyBounce = Bounce<MyCaster, Args...>;
     *
     * In more complex conversion cases, the Converter::cast
     * function will have to be defined explicitly, with 
     * internal typedefs of to_type and from_type.
     */     
    template<typename To, typename From>
    struct Caster
    {
        using to_type   = To;
        using from_type = From;
        
        static to_type* cast( from_type* from )
        {
            return reinterpret_cast<to_type*>(from);
        }
    };
    
    template<typename Converter, typename... Args>
    struct Bounce
    {
        using To   = typename Converter::to_type;
        using From = typename Converter::from_type;
        
        template<typename Retval, Retval (To::* Method)(Args...)>
        static Retval rv_fn( From* from, Args... args )
        {
            return (Converter::cast( from )->*Method)( std::forward<Args>(args)... );
        }

        template<void (To::* Method)(Args...)>
        static void vd_fn( From* from, Args... args )
        {
            (Converter::cast( from )->*Method)( std::forward<Args>(args)... );
        }
    };

} // namespace Utility
