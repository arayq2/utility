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

    template<typename ErrorPolicy = IgnoreSigwaitError>
    class SigWait
    {
    public:
        explicit
        SigWait(bool deferHandlers = false, ErrorPolicy const& errPolicy = ErrorPolicy())
        : errPolicy_(errPolicy)
        , sigmask_()
        {
            if ( !deferHandlers ) { install_handlers(); }
        }
        
        static void install_handlers( bool childAlso = false );
        
        int wait() { return sigmask_.wait( errPolicy_ ); }
        
    private:
        ErrorPolicy     errPolicy_;
        SigMask         sigmask_;
    };
    
} // namespace Utility

