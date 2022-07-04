/** ======================================================================+
 + Copyright @2020-2022 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/

#include "utility/SigWait.h"
#include "utility/StackTrace.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{
    void sigsegv_handler( int, siginfo_t*, void* ) { throw Utility::TrapException("SIGSEGV at:\n"); }
    void sigbus_handler( int, siginfo_t*, void* )  { throw Utility::TrapException("SIGBUS at:\n");  }
    void sigfpe_handler( int, siginfo_t*, void* )  { throw Utility::TrapException("SIGFPE at:\n");  }
    void sigill_handler( int, siginfo_t*, void* )  { throw Utility::TrapException("SIGILL at:\n");  }

    void do_nothing( void*, pid_t, int ) {}

    Utility::SigChldCallback    sccb = do_nothing; // default setting
    void*                       cbarg = nullptr;

    void sigchld_handler( int, siginfo_t*, void* )
    {
        pid_t   _pid;
        int     _status;

        while ( (_pid = ::waitpid( -1, &_status, WNOHANG )) > 0 ) { (*sccb)( cbarg, _pid, _status ); }
    }

} // anonymous namespace 

namespace Utility
{

    SigMask::SigMask()
    {
        ::sigemptyset( &mask_ );
        ::sigaddset( &mask_, SIGINT );
        ::sigaddset( &mask_, SIGTERM );
        ::sigaddset( &mask_, SIGHUP );
        ::sigaddset( &mask_, SIGQUIT );
        ::sigaddset( &mask_, SIGCONT );
        pthread_sigmask( SIG_BLOCK, &mask_, nullptr );
    }

    /* static */
    void SigWait::set_sigchld_cb( SigChldCallback cb, void* arg )
    {
        sccb  = cb == nullptr ? do_nothing : cb;
        cbarg = cb == nullptr ? nullptr : arg;
    }

    /* static */
    void SigWait::install_handlers( bool childAlso )
    {
        struct sigaction    _action;
    
        ::sigemptyset( &_action.sa_mask );
        _action.sa_flags = 0;
    
        // convert SIGPIPE to inband EPIPE error
        _action.sa_handler = SIG_IGN;
        ::sigaction( SIGPIPE, &_action, nullptr );
    
        // convert traps to exceptions
        _action.sa_flags = SA_SIGINFO;
    
        _action.sa_sigaction = sigsegv_handler;
        ::sigaction( SIGSEGV, &_action, nullptr );
    
        _action.sa_sigaction = sigbus_handler;
        ::sigaction( SIGBUS, &_action, nullptr );
    
        _action.sa_sigaction = sigfpe_handler;
        ::sigaction( SIGFPE, &_action, nullptr );
    
        _action.sa_sigaction = sigill_handler;
        ::sigaction( SIGILL, &_action, nullptr );
    
        if ( childAlso )
        {
            _action.sa_sigaction = sigchld_handler;
            ::sigaction( SIGCHLD, &_action, nullptr );
        }
    }

    /* static */
    void SigWait::ignore_children()
    {
        struct sigaction    _action;

        ::sigemptyset( &_action.sa_mask );
        _action.sa_flags   = SA_NOCLDWAIT | SA_NOCLDSTOP;
        _action.sa_handler = SIG_IGN;
        ::sigaction( SIGCHLD, &_action, nullptr );
    }

    /* static */
    void SigWait::die( bool crash )
    {
        ::kill( ::getpid(), crash ? SIGABRT : SIGTERM );
    }

} // namespace Utility
