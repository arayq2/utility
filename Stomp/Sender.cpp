/** ======================================================================+
 + Copyright @2023-2024 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#include "StompAgent.h"
#include "StrFile.h"

#include <iostream>

#include <cstring>

    int main( int ac, char* av[] )
    {
        if ( ac < 3 )
        {
            std::cerr << "Usage: " << av[0] << " t|q <target> [<file>... (or from STDIN)]" << std::endl;
            return 1;
        }

        bool                _isq(av[1][0] == 'q');

        Stomp::StompAgent   _agent(true);
        Stomp::EndPoint     _target{av[2], _isq};

        auto                _lambda([&]( char const* file ) -> void
        {
            Utility::StrFile    _msg(file);
            if ( !_msg )
            {
                std::cerr << "Error: " << ::strerror( _msg.error() ) << "\n";
                return;
            }
            _agent.publish( _target, _msg.get() );
        });

        if ( ac < 4 )
        {
            std::string     _line;
            while ( std::getline( std::cin, _line ) )
            {
                _lambda( _line.c_str() );
            }
        }
        else
        for ( auto _ptr(av + 3); *_ptr; ++_ptr )
        {
            _lambda( *_ptr );
        }

        return 0;
    }
