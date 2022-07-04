/** ======================================================================+
 + Copyright @2020-2022 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
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

    using SigChldCallback = void (*)( void*, pid_t, int );

    class SigWait
    {
    public:
        explicit
        SigWait(bool deferHandlers = false)
        : sigmask_()
        {
            if ( !deferHandlers ) { install_handlers(); }
        }
        
        static void set_sigchld_cb( SigChldCallback cb, void* arg = nullptr ); // nullptr = default
        static void install_handlers( bool childAlso = false );
        static void ignore_children();
        static void die( bool crash = false );
        
        template<typename ErrorPolicy = IgnoreSigwaitError>
        int wait( ErrorPolicy const& errPolicy = ErrorPolicy() ) { return sigmask_.wait( errPolicy ); }
        template<typename ErrorPolicy = IgnoreSigwaitError>
        int wait_ex( ErrorPolicy const& errPolicy = ErrorPolicy() )
        {
            int _sig;
            do { _sig = sigmask_.wait( errPolicy ); } while ( _sig == SIGCHLD );
            return _sig;
        }

    private:
        SigMask         sigmask_;
    };
    
} // namespace Utility

