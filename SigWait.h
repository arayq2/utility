#pragma once

#pragma once

#include <cerrno>
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>

namespace Utility
{
    struct IgnoreSigwaitError
    {
        void onError( int errnum ) const {}
    };
    
    class SigMask
    {
    public:
        SigMask();

        template<typename ErrPolicy>
        int wait( ErrPolicy& policy )
        {
            int     _sig;
            
            errno = 0;
            while ( ::sigwait( &mask_, &_sig ) )
            {
                policy.onError( errno );
                errno = 0;
            }
            return _sig;
        }

    private:
        sigset_t    mask_;
    };

    class SigWait
    {
    public:
        explicit
        SigWait(bool deferHandlers = false)
        : sigmask_()
        {
            if ( !deferHandlers ) { install_handlers(); }
        }
        
        static void install_handlers( bool childAlso = false );
        
        template<typename ErrorPolicy = IgnoreSigwaitError>
        int wait( ErrorPolicy const& errPolicy = ErrorPolicy() ) { return sigmask_.wait( errPolicy ); }
        
    private:
        SigMask         sigmask_;
    };
    
} // namespace Utility

