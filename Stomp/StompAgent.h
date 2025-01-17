/** ======================================================================+
 + Copyright @2020-2024 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#pragma once

#ifndef AMS_STOMPAGENT_H
#define AMS_STOMPAGENT_H

#include "StompImpl.h"

    /**
     * @file: StompAgent.h
     * @brief: STOMP client for ActiveMQ message broker
     */

namespace Stomp
{

// Stomp protocol port
#define DEFAULT_AMQBROKER   "tcp://localhost:61613"

namespace
{
    char const* safe_getenv( char const* key, char const* alt )
    {
        char const* _ptr(::getenv( key ));
        return _ptr ? _ptr : alt;
    }

    char const* amqBrokerUrl(safe_getenv("AMQBROKER_URL", DEFAULT_AMQBROKER));
}

    struct Protocol
    {
        std::string     scheme_;
        std::string     host_{"localhost"};
        int             port_{61613};

        ~Protocol() noexcept = default;
        Protocol(std::string const& url = amqBrokerUrl)
        {
            if ( !url.empty() )
            {
                auto    _pos1(url.find( ":" ));
                scheme_ = url.substr( 0, _pos1 );
                _pos1  += url.compare( _pos1 + 1, 2, "//" ) == 0 ? 3 : 1;
                auto    _pos2(url.find( ":", _pos1 ));
                host_   = url.substr( _pos1, _pos2 - _pos1 );
                port_   = std::stoi( url.substr( _pos2 + 1 ) );
            }
        }
    };

    struct Credentials
    : Protocol
    {
        std::string         user_{""};
        std::string         pass_{""};
        //
        ~Credentials() = default;
        Credentials() = default;
        Credentials(std::string const& broker, std::string const& user, std::string const& pass)
        : Protocol(broker)
        , user_(user)
        , pass_(pass)
        {}
        Credentials& user( std::string const& user ) { user_ = user; return *this; }
        Credentials& pass( std::string const& pass ) { pass_ = pass; return *this; }
        Credentials& uspw( std::string const& user, std::string const& pass )
                                                     { user_ = user; pass_ = pass; return *this; }
    };

    class StompAgent
    {
    public:
        ~StompAgent();

        explicit
        StompAgent(bool startnow = false, Credentials const& = Credentials());

        bool start();                            // start dispatch in another thread
        bool start( EndPoint const&, Callback ); // start dispatch here

        bool subscribe( EndPoint const& source, Callback handler );
        bool unsubscribe( EndPoint const& source );

        bool publish( EndPoint const& target, std::string const& message );

    private:
        Credentials     cred_;
        Connection      conn_;
        Session         sess_;
    };

} // namespace Stomp


#endif // AMS_STOMPAGENT_H

