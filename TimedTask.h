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
     * The resource-specific functionalities are in the static
     * methods of a template class argument.
     * init_value : initialized resource result type.
     * null_value : "failure" mode of the result type.
     * acquire    : obvious functionality.
     * release    : equally obvious functionality.
     *
     * NOTES: 
     *   - result type must have value semantics (e.g., pointer).
     *   - acquire method must not block indefinitely.
     *   - object must persist until producer thread completes.
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
        : TimedTask()
        {
            std::thread(std::ref(*this), std::forward<Args>(args)...)
            .detach();
        }
        
        // producer thread
        template<typename... Args>
        void operator()( Args&&... args )
        {
            Methods::acquire( result_, std::forward<Args>(args)... );
            if ( window_.is_cancelled() ) { Methods::release( result_ ); }
        }
        
        // consumer thread; effective unit: milliseconds
        Result wait( unsigned seconds, unsigned multiplier = 1000 )
        {
            return window_.is_completed( multiplier * seconds )
            ? result_
            : Methods::null_value()
            ;
        }
        
        bool reset( bool force = false )
        {
            if ( window_.reset( force ) )
            {
                result_ = Methods::init_value();
                return true;
            }
            return false;
        }

    private:
        TimedEvent      window_;
        Result          result_;
    };
    
} // namespace Utility

    