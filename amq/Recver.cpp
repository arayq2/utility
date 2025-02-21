/** ======================================================================+
 + Copyright @2020-2025 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#include "MessageReceiver.h"
#include "SigWait.h"
#include <iostream>
#include <unistd.h> // getopt

    /**
     * @class MyClient
     * Demonstrates use of mixin superclass
     */
class MyClient
: public MessageReceiver<MyClient>
{
public:

    ~MyClient() = default;
    MyClient(bool release = false)
    : MessageReceiver<MyClient>(release)
    {}
    
    void on_message( std::string const& msg, std::string const& )
    {
        std::cerr << "MyClient received: " << msg << std::endl;
    }
};

//-------------------------------------------------------------------------
    int on_error( char const* msg )
    {
        std::cerr << "Error: " << msg << std::endl;
        return 1;
    }

    struct ErrnoPolicy
    {
        void onError( int errnum ) const
        {
            std::cerr << "Errno: " << errnum << std::endl;
        }
    };

    int
    process_args( int ac, char* av[], bool& isT, bool& rel )
    {
        int             _opt;
        isT = true; // set a default
        while ( (_opt = ::getopt( ac, av, "qrt" )) != -1 )
        switch ( _opt )
        {
        case 'q': isT  = false;
        case 'r': rel  = true;
        case 't': isT  = true;
        default: break;
        }
        //
        return ::optind;
    }

    int main( int ac, char* av[] )
    {
        bool                _isTopic{false};
        bool                _release{false};
        int                 _start{process_args( ac, av, _isTopic, _release)};
        if ( ac <= _start ) { return on_error( "Not enough arguments" ); }
        //
        Utility::SigWait::install_handlers();
#ifdef DEFAULT_IMPL
        MessageReceiver     _rcl{_release};
#else
        MyClient            _rcl{_release};
#endif
        for ( ; _start < ac; ++_start ) { _rcl.subscribe( av[_start], _isTopic ); }
        Utility::SigWait(true).wait( ErrnoPolicy() );
        return 0;
    }
