#pragma once

#include "TimedEvent.h"
#include <thread>      // std::thread
#include <functional>  // std::ref
#include <utility>     // std::forward

namespace Utility
{
    /**
     * TimedTask.
     * Manages acquisition of a resource subject to a timeout.
     * The resource-specific functionality is in the static
     * methods of a template class.
     * init_value : initialized resource result type
     * null_value : "failure" mode of the result type
     * acquire    : obvious functionality
     * release    : equally obvious functionality     
     */
    template<typename Methods>
    class TimedTask
    {
    public:
        using Result = typename Methods::result_type;
        
        TimedTask()
        : result_(Methods::init_value())
        {}
        
        template<typename... Args>
        TimedTask(Args&&... args)
        : result_(Methods::init_value())
        {
            std::thread(std::ref(*this), std::forward<Args>(args)...).detach();
        }
    
        template<typename... Args>
        void operator()( Args&&... args )
        {
            Methods::acquire( result_, std::forward<Args>(args)... );
            if ( event_.expired() ) { Methods::release( result_ ); }
        }

        Result wait( unsigned seconds )
        {
            return event_.complete( 1000 * seconds )
            ? result_
            : Methods::null_value()
            ;
        }

    private:
        TimedEvent      event_;
        Result          result_;
    };
    
} // namespace Utility

    