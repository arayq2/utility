/** ======================================================================+
 + Copyright @2023-2024 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#include "StompAgent.h"
#include "Notice.h"

#include <iostream>

    class Receiver
    {
    public:
        ~Receiver() { agent_.unsubscribe( source_ ); }

        Receiver(Stomp::StompAgent& agent, Utility::Notice& notice, Stomp::EndPoint const& source )
        : agent_(agent)
        , notice_(notice)
        , source_(source)
        , callback_(Stomp::make_callback( &Receiver::on_message, this ))
        {
            agent_.subscribe( source_, callback_ );
        }

        void on_message( std::string const& message, Stomp::EndPoint const& endpoint )
        {
            std::cout << "[(" << (endpoint.isQ_ ? "Q)" : "T)") 
                      << endpoint.dest_ << "]\n"
                      << message << std::endl;            
            notice_.deliver();
        }

    private:
        Stomp::StompAgent&      agent_;
        Utility::Notice&        notice_;
        Stomp::EndPoint const&  source_;
        Stomp::Callback         callback_;
    };

    int main( int ac, char* av[] )
    {
        if ( ac < 3 )
        {
            std::cerr << "Usage: " << av[0] << " t|q <source>" << std::endl;
            return 1;            
        }

        bool                _isq(av[1][0] == 'q');

        Stomp::StompAgent   _agent(true);
        Utility::Notice     _notice;
        Stomp::EndPoint     _source{av[2], _isq};

        Receiver            _rcvr(_agent, _notice, _source);

        _notice.wait();
        return 0;
    }
