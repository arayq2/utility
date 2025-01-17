/** ======================================================================+
 + Copyright @2023-2024 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#include "StompAgent.h"

#include <iostream>

    void on_message( std::string const& message, Stomp::EndPoint const& endpoint )
    {
        std::cout << "[(" << (endpoint.isQ_ ? "Q)" : "T)") 
                  << endpoint.dest_ << "]\n"
                  << message << std::endl;            
    }

    int main( int ac, char* av[] )
    {
        if ( ac < 3 )
        {
            std::cerr << "Usage: " << av[0] << " t|q <source>" << std::endl;
            return 1;            
        }

        bool                _isq(av[1][0] == 'q');

        Stomp::StompAgent   _agent;
        Stomp::EndPoint     _source{av[2], _isq};

        _agent.start( _source, on_message );

        return 0;
    }
