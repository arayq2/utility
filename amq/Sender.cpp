/** ======================================================================+
 + Copyright @2020-2025 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#include "AmqAgent.h"
#include "IteratorRange.h"
#include "StrFile.h"
#include <iostream>

    using namespace ams;

    int on_error( char const* msg )
    {
        std::cerr << "Error: " << msg << "\n";
        return 1;
    }

    /**
     * Sender
     * Program to send messages to a destination,
     * Messages are either contents of files or separate lines from STDIN
     */
    int main( int ac, char* av[] )
    {
        if ( ac < 3 ) { return on_error( "Not enough arguments ( -q|-t target [file...] (or lines from STDIN))"); }
        if ( av[1][0] != '-' ) { return on_error( "First argument must be -q or -t" ); }

        bool        _isQ{av[1][1] == 'q'};
        AmqAgent    _agent{Credentials()};
        if ( ac < 4 )
        {
            std::string     _line;
            while ( std::getline( std::cin, _line ) )
            {
                _agent.publish( {av[1], _isQ}, _line );
            }
        }
        else
        for ( auto& _file : Utility::IteratorRange<char**>(av + 2, av + ac) )
        {
            Utility::StrFile    _msg{_file};
            if ( !_msg ) { return on_error( ::strerror( _msg.error() ) ); }

            _agent.publish( {av[2], _isQ}, _msg.get() );
            // pause?
        }
        return 0;
    }
